
#include<stdio.h>
#include"md5.h"
#include<errno.h>
#include"server.h"

int recetag = 0;

void do_md5(char * md5sources,char * md5recieve)
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
    strcpy(md5recieve,hex_output);
	return;

}

static void rece_timeout_func(int signo)
{
	recetag = 1;
	return;
}


/*   发送消息到服务器
 *   interver:时间间隔, sndmsg:消息内容, time:发送次数，如果interval为空,则取随机时间，
 */
int sndmsg(int intervals,char *sndmsg,int times,char *serverip,int serverport,char *recebuf) {  
      int len,socket_fd;  
	  int result,i;
	  int interti,ti;       //依次是时间间隔和发送次数
	  

	  interti=intervals;
	  if (!interti>0)
	  {
		  interti=1;
	  }
	  ti=times;
      socket_fd=openClientSocket(serverip,serverport);
         if (!socket_fd||socket_fd<0)
               perror("create socket err.");
      
      for (i=0;i<ti;i++)
      {
           result=send(socket_fd,sndmsg,strlen(sndmsg),0);
	       if (!result==strlen(sndmsg))
	          {
		        perror("send msg length err.");
				exit(0);
	          }
	  }//endfor
	  signal(SIGALRM, rece_timeout_func);

	  while (1)
	  {
		  alarm(1);
		  result = recv(socket_fd,recebuf,1024,0);
		  if (result >0||recetag==1)
		  {
			  break;
		  }
		  
	  }
	  close(socket_fd);
	  if (result < 0 )
	  {
		  return result;
	  } 
	  return 0;

     
	  
}

//int recemsg(char * recebuf,)
int main(int argc, char **argv){
     char serip[]="61.139.126.202";
     int serport=8009;
     char testdom[]="ebaidu.com";
     char snd[1024]={0};
	 char md5result[16*2+1]={0};
     char recebuf[10240]={0};
	 /*
	  *  Act的操作一共有 add del mod  
	  *  参数分别是: 操作类型 域名 解析类型 主机 解析值
	  */

     if (argc<2)
     {
		 printf("useage:%s 参数分别是: 操作类型 域名 解析类型 主机 解析值\r\n",argv[0]);
		 exit(-1);
     }else if (argc==3)
     {
		 //测试添加删除域名
         sprintf(snd,"%s\r\nentityname:dnsdomain\r\ndomainname:%s\r\ndnsservername:ns1.myhostadmin.net\r\n",argv[1],argv[2]);
	     do_md5(snd,md5result);
         sprintf(snd,"%s\r\nentityname:dnsdomain\r\ndomainname:%s\r\ndnsservername:ns1.myhostadmin.net\r\nkeymd5:%s\r\n.\r\n",argv[1],argv[2],md5result);
	     sndmsg(0,snd,1,serip,serport,recebuf);

     }else if (argc==6)
     {
		 //测试添加删除域名记录
         sprintf(snd,"%s\r\nentityname:dnsrecord\r\ndomainname:%s\r\nresolvetype:%s\r\nresolvehost:%s\r\nresolvevalue:%s\r\nmxlevel:10\r\n",argv[1],argv[2],argv[3],argv[4],argv[5]);
		 do_md5(snd,md5result);
		 sprintf(snd,"%s\r\nentityname:dnsrecord\r\ndomainname:%s\r\nresolvetype:%s\r\nresolvehost:%s\r\nresolvevalue:%s\r\nmxlevel:10\r\nkeymd5:%s\r\n.\r\n",argv[1],argv[2],argv[3],argv[4],argv[5],md5result);
		 sndmsg(0,snd,1,serip,serport,recebuf);

	}else{
	    sprintf(snd,"%s\r\n.\r\n",argv[1]);
        sndmsg(0,snd,1,serip,serport,recebuf);
	
	}
     
	 
	 printf("-==============================================================-\n%s",recebuf);
     


	 return 0;    
}

//2013/6/19 15:05 
