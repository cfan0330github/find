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
		  logs=(char *)malloc(sizeof(char)*1000); //һ��150
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


    char serverip[16];                                                                 //������IP��ַ
	time_t creatime;                                                                   //���η���ʧ��ʱ��
	time_t modtime;                                                                    //���һ�η���ʧ��ʱ��
	char *domain;                                                                      //������������
	char act[3];                                                                       //������ʽ:add,del,mod
	int status;                                                                        //��ͷָ��:(0:�ɲ���,1���ܲ���),Ԫ��(2��ʱ���,3��ʱɾ��)
	char md5key[33];                                                                   //��¼ΨһID
	struct _CMDLOG *nextlog;                                                           //������¼λ��