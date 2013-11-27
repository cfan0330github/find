#include <openssl/rsa.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "md5.h"
#include "server.h"


#define	QUEUE_MAX_NUM	200
#define	SSLREADTIMEOUT	30
#define SSLREADERRTIMES	60
#define ALARMTIME 5         //定时检查服务器连通性问题



FILE	*f_log, *f_dat, *f_errlog;

serverlist        *rootserverlist, *pserverlist, *tailserverlist;                            //分别为服务器ip头,当前,尾指针
cmdlog            *rootcmdlog, *pcmdlog, *tailcmdlog;                                        //分别为日志头,当前,尾指针
int cron_tag;                                                                   
 
int	current_queue_num = 0, time_out_flag = 0, kill_flag = 0;
char  *linux_dns(char * execute_buf);

extern servermode serverglobal;

void writeErrlog(FILE *f_errLog, char *strFormat, ...)
{
	va_list ap;
	struct	tm	*now_tm;
	time_t	now_time;

	va_start(ap, strFormat);
	now_time = time(0L);
	now_tm = localtime(&now_time);
	//fprintf(f_errLog,  "%04d-%02d-%02d %02d:%02d:%02d Server Infomation \n",
	//		now_tm->tm_year+1900, now_tm->tm_mon+1, now_tm->tm_mday, now_tm->tm_hour, now_tm->tm_min, now_tm->tm_sec);
	vfprintf(f_errLog, strFormat, ap);
	//fprintf(f_errLog, "\n");
	fflush(f_errLog);
	va_end(ap);
}

int checkUserPasswd(char *strUserID, char *strPasswd)
{
	if(strUserID == NULL || strPasswd == NULL)
	{
		return 0;
	}

	if(!strcmp(strUserID, USERID) && !strcmp(strPasswd, PASSWORD))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
//-=====================================================================================
/*
 *  业务处理后台常驻进程
 *  接收消息添加删除域名,识别自身身份,转发命令到slaveDNS服务器,发送失败日志接收，打印日志
 */
void initProcessor(char *serverlst)     
{
	int result,i,result_msg;                          //依次是消息大小,接收消息计数器,转发消息结果
	char *resultp;
	char *infind;
	char return_result[1024]={0};
	char fbuf[512];                                   //文件缓冲区
	char *charn;                                      //“\n”指针位置
	msgbuf rmsg;
	pid_t ppid;                                       //监视父进程是否存在
	struct msqid_ds  qidds;                           //查看消息队列状态
	int sndcmd_tag=1;                                 //是否转发指令
	
    
	NEWDOM     *root;       //,*p,*tail;                                                         //添加域名头,当前,尾指针
	NEWDOM     *delroot;    //,*delp,*deltail;                                                   //删除域名头指针,当前,尾指针

	
	FILE *f_serlst;                                  
    
	serverglobal.sl=&rootserverlist;
	serverglobal.cg=&rootcmdlog;
    cron_tag=0;
    //初始化域名列表,服务器列表,日志列表
    root=(NEWDOM *)malloc(sizeof(NEWDOM));                         //添加域名列表头
	delroot=(NEWDOM *)malloc(sizeof(NEWDOM));
	rootserverlist=(serverlist *)malloc(sizeof(serverlist));
    rootcmdlog=(cmdlog *)malloc(sizeof(cmdlog));
    infind=NULL;
	
    
	 if (!root||!rootserverlist||!rootcmdlog)
    {
		writeLog(f_log,"Memory init Failure,deamon exit.");
		exit(0);
    }

    strcpy(root->mtext,"addlist");
	root->next=NULL;
	root->tag=0;

    strcpy(delroot->mtext,"dellist");
	delroot->next=NULL;
    delroot->tag=0;

	rootcmdlog->nextlog=NULL;
    strcpy(rootcmdlog->serverip,"root");

	if (serverlst)                                                       //建立转发服务器列表,master模式
    {
          f_serlst=fopen(serverlst, "r");
		  if (f_serlst>0)
		  {
			   strcpy(rootserverlist->serip,"root");                              //服务器IP头
               rootserverlist->nextip=NULL;
			   rootserverlist->tag=0;
			   tailserverlist=rootserverlist;

			   //while(!feof(f_serlst))
		        //{
			      // fgets(fbuf,512,f_serlst);
                 while(fgets(fbuf,512,f_serlst)!=NULL) {
				   if (strlen(fbuf)<2||!strstr(fbuf,".")||strlen(fbuf)>17)
				    {
                      continue;                          //跳过非法字符
					}
					if (charn=strstr(fbuf,"\n"))
					{
                         *charn = 0;
					}

                    pserverlist=(serverlist *)malloc(sizeof(serverlist));
                    if (pserverlist)
                    {
						strcpy(pserverlist->serip,fbuf);
						pserverlist->nextip=NULL;
						pserverlist->tag=0;

						tailserverlist->nextip=pserverlist;  //连接
						tailserverlist=pserverlist;
						bzero(fbuf,512);
                    }
		        }//endwhilefeof
			  sndcmd_tag=1;	
			  writeLog(f_log,"Server mode Master sendcmd.\n");
			  fclose(f_serlst);
		  }	 else {
              writeLog(f_log,"Server mode Master,but can not open %s,exit.\n",serverlst);
              exit (0);
		     }
    }else {
	          sndcmd_tag=0;                                                   //不转发命令slave模式
			  free(rootcmdlog);
			  free(rootserverlist);
              writeLog(f_log,"Server mode Slave.\n");
	}//end serverlst      
    
    while(1)
	   {
		   i=0;    //初始化计数器
	       bzero((char *)&rmsg,sizeof(rmsg));
           
		     if (rootcmdlog->nextlog && sndcmd_tag && !rootserverlist->tag)
		     {
			   signal(SIGALRM,cron_check);                                 //定时检查连通性
			   alarm(ALARMTIME);
			   rootserverlist->tag=1;
			   //perror("set alarm ready.\n");
		     } 
 
           result=msgrcv(qid,&rmsg,sizeof(rmsg)-sizeof(long),1,0);
		   
	       if (result<0)
	          {
                  //writeErrlog(f_errlog,"rec msg err,msg length lt 0");
				  continue;                                             //收到错误消息，跳过
	          }
			  if (rmsg.act==2)
			  {
				  //debug show command
				  int nu;
				  travelCmdlog(rootcmdlog);
				  //writeErrlog(f_errlog,"cmdlog send fail list\n%s",resultp);
				  //writeErrlog(f_errlog,"cmdlog send fail list\n%d",nu);
				  //if (resultp) free(resultp);
				  //resultp=NULL;
				  continue;
			  }
    /*
	 *      收到消息做转发,添加在此=================================================================
 	 */            
				if (sndcmd_tag)  sndCmd(rootcmdlog,rootserverlist,&rmsg);
    /*
	 *      收到消息做转发,添加在此==================================================================

 	 */
	
		      if (rmsg.act==0){
                    linked(delroot,&rmsg);
			     }
              if (rmsg.act==1)
                 {
				    linked(root,&rmsg);
                 }
		     //状态设置一起读完
             for(;result>0 ;){		        
				    bzero((char *)&rmsg,sizeof(rmsg));
					result=msgrcv(qid,&rmsg,sizeof(rmsg)-sizeof(long),1,IPC_NOWAIT);
					//>0 <0 ==0
                    if (result>0)
                    {
						   if (sndcmd_tag)  sndCmd(rootcmdlog,rootserverlist,&rmsg);     //转发数据
						   if (rmsg.act==1)                                              //此为添加域名，需要批量添加
						   {
							    linked(root,&rmsg); 
					        }else if (rmsg.act==0)          //删除域名
					        {
							    linked(delroot,&rmsg);
					        }else { 
						      continue;                     //未知指令，直接跳过
						    }
					}//end if(result)
					
              }//end for
			 // tail=NULL;
            
            if (root->next)
            {
				checkzone(root);                       //检查域名有效性
				resultp=addDomain(root,sndcmd_tag);    //批量添加到配置文件
			    strcpy(return_result,resultp);
                writeLog(f_log,"add domain \r\n%s",return_result);  
            }                       
            //printf("add success one time!\r\n");
             if (delroot->next)
             {
				 resultp=delDomain(delroot);           //删除域名
				 strcpy(return_result,resultp);
                 writeLog(f_log,"del domain \r\n%s",return_result);
             }
		  /* result=msgctl(qid,IPC_STAT,&qidds);
	       if(!result){
		       ppid=getppid();
	           if(qidds.msg_qnum==0 && ppid==1)
		          {
		             exit (0);    //进程变成独立进程并且队列没有消息,自动退出.
		           }	
	         }
            
 处理业务
			if (rmsg.dominfo.act=1){   //添加域名
                result=addNewDomain(rmsg.dominfo.mtext,rmsg.dominfo.ttl);

			}else if (rmsg.dominfo.act=0)        //删除域名
			{
				result=delDomain(rmsg.dominfo.mtext);

			}
			*/	                 	   
				  
	   }//end while
}//end func Initprocessor()

static void read_timeout_func(int signo)
{
	time_out_flag = 1;
	return;
}

void writeLog(FILE *fLog, char *strFormat, ...)
{
	va_list ap;
	struct	tm	*now_tm;
	time_t	now_time;

	va_start(ap, strFormat);
	now_time = time(0L);
	now_tm = localtime(&now_time);
	fprintf(fLog,  "%04d-%02d-%02d %02d:%02d:%02d\n",
			now_tm->tm_year+1900, now_tm->tm_mon+1, now_tm->tm_mday, now_tm->tm_hour, now_tm->tm_min, now_tm->tm_sec);
	vfprintf(fLog, strFormat, ap);
	fprintf(fLog, "\n");
	fflush(fLog);
	va_end(ap);
}

char *BURRP_session(const char *execute_buf, int *result, int *intAuth)
{
	int i_ret;
	char *strUserID, *strPasswd , *tempstring;

	char	*str_value;
	char *return_result;

	*intAuth = 0;
	str_value = (char *)malloc(strlen(execute_buf) + 100);

	if(str_value == NULL)
	{
		return_result = (char *)malloc(strlen(REGMSG_421) + 1);
		if(return_result != NULL)
		{
			strcpy(return_result,REGMSG_421);
		}
		*result = 1;
		return return_result;
	}

	//把那个字符串的前多少位拷贝给1个数字。就那么简单了,然后再i_ret这个位置复制伪'\0'

    sprintf(str_value,"%s",execute_buf);
	if (!strstr(execute_buf,"keymd5"))
	{
         return "cmd";
	}

	i_ret = strlen(execute_buf) - strlen(strstr(execute_buf,"keymd5"));

	strUserID = (char *)malloc(strlen(execute_buf) + 100);

	sprintf(strUserID,"%s",execute_buf);

	strUserID[i_ret]='\0';

	i_ret = 0;


	i_ret = getvalue(execute_buf, "keymd5", ":", str_value, "\r\n");
	if(i_ret)
	{
		return_result = (char *)malloc(strlen(REGMSG_504) + 32);
		if(return_result != NULL)
		{
			sprintf(return_result, REGMSG_504, "keymd5");
		}
		*result = 2;
		free(str_value);
		free(strUserID);
		return return_result;
	}
	strPasswd = (char *)malloc(strlen(str_value) + 1);
	if(strPasswd == NULL)
	{
		return_result = (char *)malloc(strlen(REGMSG_421) + 1);
		if(return_result != NULL)
		{
			strcpy(return_result,REGMSG_421);
		}
		*result = 1;
		free(str_value);
		free(strUserID);
		return return_result;
	}
	strcpy(strPasswd, str_value);
	//这里做出  strPasswd 和 keymd5 的比较:)
	//strUserID  是 keystring   // strpasswd 是keymd5
 
	if(!do_md5(strUserID,strPasswd))
	{
		return_result = (char *)malloc(strlen(REGMSG_531) + 1);
		if(return_result != NULL)
		{
			strcpy(return_result,REGMSG_531);
		}
		*result = 0;
		free(str_value);
		free(strUserID);
		free(strPasswd);
		return return_result;
	}
	else
	{
		free(str_value);
		free(strUserID);
		free(strPasswd);
		return "ok";
	}
}

bool mk_md5(char * md5sources,char * md5recieve)
{

        int i;
        int status = 0;
	md5_state_t state;
	md5_byte_t digest[16];
	char hex_output[16*2 + 1];
	int di;
	char *thisvaluetemp;
    
	if (!strlen(md5sources))
	{
		return FALSE;
	}
	thisvaluetemp = (char *)malloc(strlen(USERID) + strlen(PASSWORD) + strlen(md5sources) + 100);

	strcpy(thisvaluetemp,md5sources);
	strcat(thisvaluetemp,USERID);
	strcat(thisvaluetemp,PASSWORD);

	md5_init(&state);
	md5_append(&state, (const md5_byte_t *)thisvaluetemp, strlen(thisvaluetemp));
	md5_finish(&state, digest);
	for (di = 0; di < 16; ++di)
	    sprintf(hex_output + di * 2, "%02x", digest[di]);

	free(thisvaluetemp);
    strcpy(md5recieve,hex_output);
	return TRUE;

}

int do_md5(char * md5sources,char * md5recieve)
{

    int i;
    int status = 0;
	md5_state_t state;
	md5_byte_t digest[16];
	char hex_output[16*2 + 1];
	int di;
	char *thisvaluetemp;

	thisvaluetemp = (char *)malloc(strlen(USERID) + strlen(PASSWORD) + strlen(md5sources) + 100);

	strcpy(thisvaluetemp,md5sources);
	strcat(thisvaluetemp,USERID);
	strcat(thisvaluetemp,PASSWORD);

	md5_init(&state);
	md5_append(&state, (const md5_byte_t *)thisvaluetemp, strlen(thisvaluetemp));
	md5_finish(&state, digest);
	for (di = 0; di < 16; ++di)
	    sprintf(hex_output + di * 2, "%02x", digest[di]);

	free(thisvaluetemp);

	if(!strcmp(hex_output, md5recieve) )
	{
		return 1;
	}
	else
	{
		return 0;
	}

}


char *BURRP(char *execute_buf, int *result, int *intAuth)
{
	int	i_valid_com, i_temp_valid_com = 0, i_valid_com_len;
	char	*StrValidCom, *Strattr;
	msgbuf smsg;
	char	*valid_dom[valid_com_num] =
	{
		//========增加对未发送成功的日志查询
		"show\r\n",
		"quit\r\n",
		"describe\r\n"

	};
	char *return_result, *strTemp;

	for(i_valid_com = 0; i_valid_com < valid_com_num; i_valid_com ++){
		StrValidCom = valid_dom[i_valid_com];
		i_valid_com_len = strlen(StrValidCom);
		if(!strncasecmp(execute_buf, StrValidCom, i_valid_com_len)) {
			i_temp_valid_com = 1;
			break;
		}
	}
/*
	if(!i_temp_valid_com) {
		strcpy(return_result, REGMSG_500);
		return 0;
	}
*/

	Strattr = (char *)malloc(strlen(execute_buf) + valid_com_num);

	sprintf(Strattr,"%s",execute_buf);


	if(!(BURRP_session(Strattr, result, intAuth)=="ok"))
	{
		
		//添加程序当前日志记录,消息数目-消息传递到后端
		    bzero((char *)&smsg,sizeof(smsg));
			smsg.mtype=1;
			smsg.act=2;
			smsg.ttl=0;
			sprintf(smsg.mtext,"none");
			result=msgsnd(qid,&smsg,sizeof(smsg)-sizeof(long),IPC_NOWAIT);
            if (result<0)
            {
				return_result = (char *)malloc(strlen("msgSnd:msg send err.\n") + 1);
				strcpy(return_result,"msgSnd:msg send err.\n");
            }else if (result==0)
            {
				  return_result = (char *)malloc(strlen(REGMSG_200) + 1);
                  strcpy(return_result,REGMSG_200);
            }
		//strcpy(return_result, REGMSG_531);
		//writeLog(f_log,"%s\r\n",REGMSG_531);
		return return_result;
	}
	switch(i_valid_com) {
		case 0:		//session
			//return_result = BURRP_session(Strattr, result, intAuth);
			//这个是查看用户和密码是否对应这里不用这个了默认是表示授权失败.
			strcpy(return_result, REGMSG_531);
			break;
		case 1:		//quit
			*result = 0;
			return_result = (char *)malloc(strlen(REGMSG_200) + 1);
			if(return_result == NULL)
			{
				*result = -5;
			}
			else
			{
				strcpy(return_result, REGMSG_200);
			}
			*intAuth = 0;
			break;
		case 2:		//describe
			*result = 0;
			return_result = (char *)malloc(strlen(REGMSG_200B) + 1);
			if(return_result == NULL)
			{
				*result = -5;
			}
			else
			{
				strcpy(return_result, REGMSG_200B);
			}
			break;
		default:	//其他的到其他的程序取
			*result = 0;
			//nprintf("The values is %s\r\n\r\n",execute_buf);
			return_result = linux_dns(execute_buf);	//处理结果
			break;
	}
    free(Strattr);
	return return_result;
}

static void kill_proc(int signo)
{
	kill_flag = 1;
	return;
}



void *thread_do(int intAcceptPort)
{
	char chrBuf[MAXBUFFERLEN], rrp_result[1025], *ptrTemp;
	char *return_result;
	int result, intBufLen, intReadErrTimes = 0;
	int blnAuth = 0;	//判断一个连接是否已经是通过验证的.



	while(1) //一个连接循环读入
	{
		intBufLen = 0;
		ptrTemp = chrBuf;
		chrBuf[0] = 0;
		do
		{
			time_out_flag = 0;
			result = 0;
			signal(SIGALRM, read_timeout_func);
			alarm(SSLREADTIMEOUT);
			result = recv(intAcceptPort,ptrTemp,1024, 0);
			alarm(0);

			if(time_out_flag && result <= 0)
			{
				//如果超时没有信息并且是没经过验证的, 关掉连接
				//intReadErrTimes ++ ;
				if(!blnAuth || errno != EINTR) // || intReadErrTimes >= SSLREADERRTIMES
				{
					close(intAcceptPort);
					writeLog(f_log, "Socket read TIMEOUT. Auth %d. ErrorTimes %d\n", blnAuth, intReadErrTimes);
					exit(0);
				}
				continue;
			}

			if(result <= 0)
			{
				close(intAcceptPort);
				writeLog(f_log, "Socket read error may be the client close the socket.\n");
				exit(0);
			}
			intBufLen += result ;
			chrBuf[intBufLen] = 0;
			ptrTemp += result;
			intReadErrTimes = 0;
			if(strstr(chrBuf, "\r\n.\r\n") || intBufLen >= MAXBUFFERLEN - 1024)
			{
				break;
			}
		}while(result > 0);


		//writeLog(f_log, "Get request\n%s", chrBuf); 
		ptrTemp = strstr(chrBuf, "\r\n.\r\n");
		return_result = NULL;
		if(ptrTemp == NULL)
		{
			return_result = (char *)malloc(strlen(REGMSG_507) + 1);
			if(return_result != NULL)
			{
				strcpy(return_result, REGMSG_507);
			}
		}
		else
		{
			//lower(chrBuf);
            
			return_result = BURRP(chrBuf,&result, &blnAuth);

		}


		if(return_result != NULL)
		 {
			intBufLen = send(intAcceptPort,return_result,1024, 0);
			if(intBufLen < 0)
			{
				writeLog(f_log, "BURRP respondence. Write error may be the client close the socket.\n%s", return_result);
				close(intAcceptPort);
				exit(0);
			}
			else
			{
			   // writeLog(f_log, "Get request\n%s", chrBuf); 
				writeLog(f_log, "Get request\n%sResult %s", chrBuf,return_result);
			}
			
		}
		else
		{
			writeLog(f_log, "Get request\n%s,Server respondence. Return Result is NULL!\n",chrBuf);
			close(intAcceptPort);
			exit(0);
		}

		if(!blnAuth)
		{
			break;
		}

		kill_flag = 0;
		signal(SIGKILL, kill_proc);
		signal(SIGTERM, kill_proc);
		if(kill_flag)
		{
			break;
		}
	} //end while 1
	close(intAcceptPort);
	exit(0);
}



void wait_child(int signo)
{
	int waitstat;

	waitpid(-1, &waitstat, WNOHANG);
	/*if(current_queue_num > 0)
	{
		current_queue_num -- ;
	}*/
	return;
}

main(int argc, char **argv)
{
	struct	sockaddr_in cliaddr;
	int	listen_socket, result, client_socket, sin_size;
	int	pth_id, waitstat;
	int fancyss;
	char cliIP[16];
	char *serlstip;
//-=======================
    key_t key=17;


	if(argc < 4)
	{
		printf("Usage: %s (int)listenport logFile accesshostFile serverlistFile\n", argv[0]);
		exit(0);
	}

	if (access(argv[2], R_OK) == 0)
	{
		f_log = fopen(argv[2], "a+");
	}
	else
	{
		f_log = fopen(argv[2], "w");
	}
	if (f_log == NULL){
		printf("%s cannot open Logfile %s",argv[0],argv[2]);
		exit(-1);
	}
	f_errlog = fopen(ERRLOG, "w");
    if (f_errlog == NULL){
		printf("%s cannot open Errlogfile %s",argv[0],ERRLOG);
		exit(-1);
	}
	serlstip=NULL;
	if (argv[4])
	{
        serlstip=argv[4];
	}
/*
	if((f_dat = fopen(argv[3], "r")) == NULL) {
		perror("open accesshostFile error");
		fclose(f_log);
		exit(-1);
	}
*/
    if (fork()>0)
    {
		exit(0);
    }else {

	listen_socket = openServerSocket(atoi(argv[1]));
	if(listen_socket < 0)
	{
		//writeLog(f_log, "open listen_socket error %d\n", listen_socket);
		perror("open listen_socket error ,exit");
		exit (-1);
	}
	//writeLog(f_log, "BURRP start.........\n");
	//-=====================初始化接收队列进程，处理业务
    qid=msgget(key,IPC_CREAT|0600);    //消息队列初始化
	if (qid<0)
	{
         writeLog(f_log,"main:Create queue failue.");
		 exit (-1);
	}

    pth_id=fork();
	if (pth_id==0){
		initProcessor(serlstip);          //业务处理进程
	}else if (pth_id<0)
	{
        writeLog(f_log,"main:Fork initProcessor Err.");
	}
    
//-====================
	signal(SIGCHLD, wait_child);
	while(1)
	{
		sin_size=sizeof(struct sockaddr_in);
		client_socket = accept(listen_socket, (struct sockaddr *)&cliaddr, &sin_size);
		if(client_socket < 0)
		{
			continue;
		}
		strncpy(cliIP, inet_ntoa(cliaddr.sin_addr), 16);
		cliIP[15] = 0;
		if(!checkAccessHost(argv[3], cliIP))
		{
			close(client_socket);
			writeLog(f_log, "The IP %s is deny!\n", cliIP);
			continue;
		}

		/*if(current_queue_num >= QUEUE_MAX_NUM)
		{
			close(client_socket);
			writeLog(f_log, "Too many connection\n");
			continue;
		}*/

		waitpid(-1, &waitstat, WNOHANG);

		pth_id = fork();
		if(pth_id == 0)
		{
			close(listen_socket);

			      thread_do(client_socket);

			exit(0);
		}
		else if(pth_id < 0)
		{
			writeLog(f_log, "fork child processor error\n");
		}
		/*else
		{
			current_queue_num ++ ;
		}*/
		close(client_socket);

		kill_flag = 0;
		signal(SIGKILL, kill_proc);
		signal(SIGTERM, kill_proc);
		if(kill_flag)
		{
			break;
		}
	}
	close(listen_socket);
	}//endfork
}
//2013/7/18 08:54 