char *travelCmdlog(cmdlog* log){
	cmdlog *p;
	char *logs=NULL;
	size_t pos=count=0;
	p = log->nextlog;
	struct	tm	*c_tm,*m_tm;
  for (;p ; )
  {
	  if (!logs)
	  {
		  logs=(char *)malloc(sizeof(char)*1000); //一行150
		  if (!logs) return NULL;	
	  }
      count+ =150;
	  if (count >1000)
	  {
		  logs=realloc(logs,count);
		  if (!logs)
		  {
			  return NULL;
		  }
	  }
	  c_tm = localtime(p->createtime);
      m_tm = localtime(p->modtime);

	  sprintf(logs,"%04d-%02d-%02d %02d:%02d:%02d %s %s %s lastmod:%04d-%02d-%02d %02d:%02d:%02d\n",c_tm->tm_year+1900,c_tm->tm_mon+1,c_tm->tm_mday,\
		         c_tm->tm_hour,c_tm->tm_min,c_tm->tm_sec,p->serverip,p->domain,p->act, \
		         m_tm->tm_year+1900,m_tm->tm_mon+1,m_tm->tm_mday,m_tm->tm_hour,m_tm->tm_min,m_tm->tm_sec);
	  p=p->nextlog;
  }
  return logs;
}


    char serverip[16];                                                                 //服务器IP地址
	time_t creatime;                                                                   //初次发送失败时间
	time_t modtime;                                                                    //最近一次发送失败时间
	char *domain;                                                                      //出错域名名称
	char act[3];                                                                       //操作方式:add,del,mod
	int status;                                                                        //仅头指针:(0:可操作,1不能操作),元素(2临时添加,3临时删除)
	char md5key[33];                                                                   //记录唯一ID
	struct _CMDLOG *nextlog;                                                           //下条记录位置