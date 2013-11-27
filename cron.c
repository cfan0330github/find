#include"server.h"

extern cmdlog *rootcmdlog;
extern serverlist *rootserverlist;
extern int f_errlog;
extern int cron_tag;
//-===============================================-
/*  
 * 定时先检查服务器的连通性,再重发命令
 */
void cron_check(int signo) {
	 serverlist *psl;
     int socket_fd,isok_tag;
     cmdlog *p;
	 char iplst[16]={0};
	 char s_msg[MAXMSGSIZE]={0};
	 char md5key[33]={0};
	 msgbuf *tmpbuf;
	 
	 int result=0;
	 isok_tag=0;
	 psl=rootserverlist->nextip;
     p=rootcmdlog->nextlog;
     
	 rootserverlist->tag=0;

     if (p)
     {
		 for(;psl;){
			 if (psl->tag)
			 {
				socket_fd=openClientSocket(psl,7890); 
				if (socket_fd>0)  {
                                   isok_tag=1,psl->tag=2;
                                   close(socket_fd);
                                  }
			 }
		     psl=psl->nextip;
		 }//endfor
     }

     if (isok_tag) {
          psl=rootserverlist->nextip;
	      for (;psl; )
	      {
			 
			  if (psl->tag==2)
			  {
				p=rootcmdlog->nextlog;
                while(p) {
					if (strstr(psl->serip,p->serverip))
					{
				        sprintf(s_msg,"%s\r\nentityname:dnsdomain\r\ndomainname:%s\r\ndnsservername:ns1.myhostadmin.net\r\n",p->act,p->domain);
				        mk_md5(s_msg,md5key);
                        sprintf(s_msg,"%s\r\nentityname:dnsdomain\r\ndomainname:%s\r\ndnsservername:ns1.myhostadmin.net\r\nkeymd5:%s\r\n.\r\n",p->act,p->domain,md5key);
                        result=sndMsg(s_msg,p->serverip,7890);
				      if (result>0) {
                        psl->tag=0;
					    
						//建立临时缓存区
						tmpbuf=(msgbuf *)malloc(sizeof(msgbuf));
						tmpbuf->ttl=0;
						tmpbuf->mtype=0;
                        if (tmpbuf)
                        {
							if ("add"==p->act)
							{
								tmpbuf->act=0;
							}else {
							    tmpbuf->act=1;
							}
                        }else {
						     writeErrlog(f_errlog,"cron_snd:malloctmpbuf err.");
						}
						strcpy(tmpbuf->mtext,p->domain);
                        if (delCmdlog(rootcmdlog,tmpbuf,p->serverip)<0)
                         {
							  perror("cron_snd:delcmdlog err.");
                         }
						 free(tmpbuf);
					} else {
					     psl->tag=1;
						 perror("still can't contect.");
					     break;
					  }
					}
					p=p->nextlog;
				}//endwhilep
			  }//endifpsl->tag
			 psl=psl->nextip;
	      }//endforpsl
     }//endifisok_tag
 
}

//-===============================================-
/*
 * cron定时向主程序发送指令，定时1小时发一次
 */ 
/*
 static void cron_snd(int signo){
	 time_t curtime,tmptime,div,timeout48;
	 cmdlog *p;
	 tmptime=0L;
	 char iplst[16]={0};
	 char s_msg[MAXMSGSIZE]={0};
	 char md5key[33]={0};
	 msgbuf *tmpbuf;
	 int result=0;
     
	 //printf("new alarm.\n");
	 p=rootcmdlog->nextlog;
	 curtime=time(0L);
	 cron_tag=1;
      if (p)         //是否有需要发送的日志命令
      {
         while(p){
		    div=curtime-p->modtime;
			timeout48=(p->modtime-p->creatime);
			if (timeout48 >= 172800)        //大于48小时，写错误日志
			{
				writeErrlog(f_errlog,"CROND:TIMEOUT %s %s %s.",p->serverip,p->act,p->domain);
			}
			if (div >= 3600)               //大于一个小时,重发命令
			{
				   sprintf(s_msg,"%s\r\nentityname:dnsdomain\r\ndomainname:%s\r\ndnsservername:ns1.myhostadmin.net\r\n",p->act,p->domain);
				   mk_md5(s_msg,md5key);
                   sprintf(s_msg,"%s\r\nentityname:dnsdomain\r\ndomainname:%s\r\ndnsservername:ns1.myhostadmin.net\r\nkeymd5:%s\r\n.\r\n",p->act,p->domain,md5key);
				   //result=sndMsg(s_msg,p->serverip,7890);
                  // if (result<0)
                  // {
			            int i=0; 
			            do{	 
				         result=sndMsg(s_msg,p->serverip,7890);
			             if (result>0) break;
				          i++;
			            }while(i<2) ; //send three time.
				//   }

                        //建立临时缓存区
						tmpbuf=(msgbuf *)malloc(sizeof(msgbuf));
						tmpbuf->ttl=0;
						tmpbuf->mtype=0;
                        if (tmpbuf)
                        {
							if ("add"==p->act)
							{
								tmpbuf->act=1;
							}else {
							    tmpbuf->act=0;
							}
                        }else {
						     writeErrlog(f_errlog,"cron_snd:malloctmpbuf err.");
						}
						strcpy(tmpbuf->mtext,p->domain);

                        //result 
						if (result<0)
					   {
			               if (modCmdlog(rootcmdlog,tmpbuf,p->serverip)<0)   //log err.
			                   {
				              writeErrlog(f_errlog,"cron_snd:writecmdlog err.");
			                    }
						
					   }else if (result>0)
					   {
                           if (delCmdlog(rootcmdlog,tmpbuf,p->serverip)<0)
                              {
							  writeErrlog(f_errlog,"cron_snd:delcmdlog err.");
                               }
					   }//endif result
					   free(tmpbuf);
			}else {
			   if (div>tmptime) tmptime=div;
			}//enddiv
		 p=p->nextlog;
		 }//endwhile
		  cron_tag=3600-div;
		  alarm((int)(cron_tag));            //设置定时器
		  printf("set new alarm at %d sec\n",(int)cron_tag);
  } //endp
  return;
}
*/
//2013/7/15 14:50 
