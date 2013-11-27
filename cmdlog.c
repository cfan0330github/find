#include<stdio.h>
#include"server.h"
#include<time.h>
#include"md5.h"

extern FILE *f_errlog;

char sourceStrmsg[MAXMSGSIZE]={0};
char md5ID[33]={0};

//0 failure;1 success;-1 error.
cmdlog* findCmdlog(cmdlog* log,msgbuf* gbuf,char* serip,int tag);

int addCmdlog(cmdlog* log,msgbuf* gbuf,char* serip);

int modCmdlog(cmdlog* log,msgbuf* gbuf,char* serip);

int delCmdlog(cmdlog* log,msgbuf* gbuf,char* serip);

void travelCmdlog(cmdlog* log);


cmdlog* findCmdlog(cmdlog* log,msgbuf* gbuf,char* serip,int tag){
	//0 none;1 find;-1 err.
      cmdlog *p,*pos;
      p=log->nextlog;
	  pos=log;
      bool ismd5=FALSE;
      
	  //log->status=1;   
      sprintf(sourceStrmsg,"%d%s%s",gbuf->act,serip,gbuf->mtext);
      ismd5=mk_md5(sourceStrmsg,md5ID);
	  bzero(sourceStrmsg,MAXMSGSIZE);
	  if (!ismd5)
	  {         
		 return NULL;
	  }else {
	     while(p) {
           if (!strcasecmp(p->md5key,md5ID))
           { 
			  //log->status=0;
			  bzero(md5ID,33);
			  if (!tag)
			  {
                 return p;
			  }else {
			     return pos;
			  }  
           }
		   pos=p;
		   p=p->nextlog;

         }//endwhile
         bzero(md5ID,33);
		 //log->status=0;
		 return NULL;
	  }
}

int addCmdlog(cmdlog* log,msgbuf* gbuf,char* serip){
   cmdlog* p,*q,*current;
   char* domainame;
   time_t	now_time;
   bool ismd5=FALSE;
    
   current=findCmdlog(log,gbuf,serip,0);
   
   if (current)
   {
      return 0;
   }else{
	  p=log;
      while(p->nextlog)
	        p=p->nextlog;
	    
        q=(cmdlog *)malloc(sizeof(cmdlog));
		if (q)
		{
          strcpy(q->serverip,serip);
		  if (gbuf->act==0)
		  {
			  strcpy(q->act,"del");
		  }else if (gbuf->act==1)
		  {
		      strcpy(q->act,"add");
		  }else {
			   free(q);
		       return -1;
		  }

          sprintf(sourceStrmsg,"%d%s%s",gbuf->act,serip,gbuf->mtext);
          ismd5=mk_md5(sourceStrmsg,md5ID);
          bzero(sourceStrmsg,MAXMSGSIZE);
	      strcpy(q->md5key,md5ID);
		  bzero(md5ID,33);
          domainame=(char *)malloc(MAXMSGSIZE*sizeof(char));
		  if(domainame){
			  bzero(domainame,sizeof(MAXMSGSIZE));
			  if (strlen(gbuf->mtext)>3) {
				  memcpy(domainame,gbuf->mtext,strlen(gbuf->mtext)+1);
				  q->domain=domainame;
			  }
		  }else{
		      free(q);
			  return -1;
		  }
		  now_time=time(NULL);
		  q->creatime=q->modtime=now_time;
          domainame=NULL;
		  q->nextlog=NULL;
		  p->nextlog=q;
		  return 1;
		}//endif q
	    return -1;
   }
}

int modCmdlog(cmdlog* log,msgbuf* gbuf,char* serip){
   
   cmdlog* p,*tail;
   char* domainame;
   time_t	now_time;
   bool ismd5=FALSE;
   
   tail=log;
   p=findCmdlog(log,gbuf,serip,0);
   while(tail){
        tail=tail->nextlog;
   }
   if (p){
       p->modtime=time(0L);
	   return 1;
   }else {
   return -1;
   }
}

int delCmdlog(cmdlog* log,msgbuf* gbuf,char* serip){
    cmdlog *p,*q;
	p=findCmdlog(log,gbuf,serip,1);

	if (p)
	{
		q=p->nextlog->nextlog;
		free(p->nextlog);
	    p->nextlog=q;
		return 1;
	}else {
	    return -1;
	}
}

void travelCmdlog(cmdlog* log){
	cmdlog *p;
	char *logs=NULL;
	char *logd=NULL;
	size_t pos,count,rem,i;
	p = log->nextlog;
	time_t	now_time;
	struct	tm	*c_tm,*m_tm,*now_tm;
	//pos=count=rem=i=0;
	char strbuf[1500];
	pos=0;
	count=0;
	rem=0;
	i=0;

    now_time = time(0L);
	now_tm = localtime(&now_time);
    writeErrlog(f_errlog,"%04d-%02d-%02d %02d:%02d:%02d Server Infomation \n",\
			now_tm->tm_year+1900, now_tm->tm_mon+1, now_tm->tm_mday, now_tm->tm_hour, now_tm->tm_min, now_tm->tm_sec);
  for (;p ; )
  {
	  c_tm = localtime(&p->creatime);
      m_tm = localtime(&p->modtime);
      memset(strbuf,0,1500);
	  
	  if (p->serverip)// && p->act && p->domain)
	               sprintf(strbuf,"DATE:%04d-%02d-%02d %02d:%02d:%02d\tSendto:%s\tACT:%s\tStatus:failure\tDomain:%s\n",\
	  c_tm->tm_year+1900,c_tm->tm_mon+1,c_tm->tm_mday,c_tm->tm_hour,c_tm->tm_min,c_tm->tm_sec,\
	  p->serverip,p->act,p->domain);
      writeErrlog(f_errlog,strbuf);
	  p=p->nextlog;
	  i++;
  }
  writeErrlog(f_errlog, "\n");
}
//2013/7/18 09:54 