
#include "server.h"
#include "pwd.h"

void getUGID(uid_t *uidNamed, gid_t *gidNamed, char *strUserName)
{
	struct passwd *pwdNamed;

	//setpwent();
	pwdNamed = getpwnam(strUserName);
	if(pwdNamed == NULL)
	{
		*uidNamed = -1;
		*gidNamed = -1;
	}
	else
	{
		*uidNamed = pwdNamed->pw_uid;
		*gidNamed = pwdNamed->pw_gid;
	}
	//endpwent();
}

 int nsupdate(char *cmd){
	//puts(cmd);
	FILE * nspipe=popen("/usr/bin/nsupdate","w");
	if (nspipe==NULL)
		return -1;
	setlinebuf(nspipe);
	if (fputs(cmd,nspipe)==EOF){
		pclose(nspipe);
		return -1;
	}
	return (pclose(nspipe));
 }

char * doUpdate(char * strDomain,int act,char *restype,char *host,char *value,char *valueold,int priority,int ttl){
	char szUpdateCmd[5046];
	char szCmdTmp[2048];
	char fqdn[1024];
	char resvalue[1024];
	char oldvalue[1024];


	if (*host=='@')
		strcpy(fqdn,strDomain);
	else
		sprintf(fqdn,"%s.%s",host,strDomain);
	
	strcpy(resvalue,value);
	if (valueold) strcpy(oldvalue,valueold);
	if (!strcmp(restype,DEFAULTMXTYPE) || !strcmp(restype,DEFAULTCNAMETYPE)){
		if (strchr(value,'.')==NULL)
			sprintf(resvalue,"%s.%s",value,strDomain);
		if (act>0 && valueold && strchr(valueold,'.')==NULL)
			sprintf(oldvalue,"%s.%s",valueold,strDomain);
	}else if (!strcmp(restype,DEFAULTTXTTYPE)){
		sprintf(resvalue,"\"%s\"",value);
		if (valueold)
			sprintf(oldvalue,"\"%s\"",valueold);
	}


	if (!strcmp(restype,DEFAULTMXTYPE)){
		strcpy(szCmdTmp,resvalue);
		sprintf(resvalue,"%d %s",priority,szCmdTmp);
		
		if (valueold){
			strcpy(szCmdTmp,oldvalue);
			sprintf(oldvalue,"%d %s",priority,szCmdTmp);
		}
	}

	bzero(szUpdateCmd,sizeof(szUpdateCmd));

	switch (act){
			case 0:
				sprintf(szUpdateCmd,"server 127.0.0.1\nupdate add %s %d %s %s\nsend\nquit\n",fqdn,ttl,restype,resvalue);					
				break;
			case 1:
				if (valueold)
					sprintf(szCmdTmp,"update delete %s %s %s",fqdn,restype,oldvalue);					
				else
					sprintf(szCmdTmp,"update delete %s %s",fqdn,restype);
				
				sprintf(szUpdateCmd,"server localhost\n%s\nupdate add %s %d %s %s\nsend\nquit\n",szCmdTmp,fqdn,ttl,restype,resvalue);					
				break;
			case 2:
				if (valueold)
					sprintf(szUpdateCmd,"server localhost\nupdate delete %s %s %s\nsend\nquit\n",fqdn,restype,oldvalue);
				else
					sprintf(szUpdateCmd,"server localhost\nupdate delete %s %s\nsend\nquit\n",fqdn,restype);								
		}

	if (nsupdate(szUpdateCmd)==0)
		return REGMSG_200;
	else
		return REGMSG_601;
}


char *strCASEstr(char *haystack, char *needle);

  int execbaiumailcmd(char *strExecCommand)
{
        char commandstring[150];
        FILE *fp;
        sprintf(commandstring, strExecCommand);
        if((fp = popen(commandstring, "r")) == NULL)
        {
                return -1;
        }
        pclose(fp);
        return 0;
}

int lockp(char *LockFile)
{
	int fileid, count = 0;

	while((fileid = open(LockFile, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR)) == -1)
	{
		if(errno != EEXIST) return CHG_ELOCKP;
		sleep(1);
		if(++count > 50)
		{
			return CHG_ELOCKP;
		}
	}
	close(fileid);
	return 0;
}

int unlockp(char *LockFile)
{
	if(unlink(LockFile))
	{
		return CHG_EULOCKP;
	}
	return 0;
}


int getDomainSerial(char *strDomainSerial)
{
                time_t t;
				struct tm *d;
				t=time(NULL);
				d=localtime(&t);
				sprintf(strDomainSerial,"%04d%02d%02d01",d->tm_year+1900,d->tm_mon+1,d->tm_mday);
                return 0;				
}

int getvalueCheckType(char *StrSource, char *StrName, char *Strequal, char *StrValue, char *StrSeparator,char *StrValuetype)
{	//本函数目的从StrSource串中取出 StrName 的值，付给StrValue; StrValue 必须为已分配足够空间
	//Strequal 为 StrSource中 StrName 与 StrValue 中的付值符号; *StrSeparator为每段(name,value)的结束符
	//成功取到值，返回0.
	char *StrTemp, *StrTemp1, *StrTemp2;
	StrValue[0] = 0;
	StrTemp = strCASEstr(StrSource, StrName);
	if(StrTemp == NULL)
		return -1;
	StrTemp += strlen(StrName);
	StrTemp1 = strstr(StrTemp, Strequal);
	if(StrTemp1 == NULL)
		return -2;
	StrTemp2 = strstr(StrTemp, StrSeparator);
	if(StrTemp2 == NULL || StrTemp2 < StrTemp1)
	{
		return -4;
	}
	StrTemp1 +=	strlen(Strequal);
	StrTemp = strstr(StrTemp1, StrSeparator);
	if(StrTemp == NULL)
		return -3;
	if(StrTemp == StrTemp1)
	{
		return -5;
	}
	strncpy(StrValue, StrTemp1, StrTemp - StrTemp1);
	StrValue[StrTemp - StrTemp1] = 0;
	if (!strcmp(StrValuetype,"int"))
	{
		if(atoi(StrValue) <= 0 )
		{
			return -6;
		}
	}
	return 0;
}


int getDomaindir(char *strDomainPath ,char *strDomain )
{
	int icount=0,domainlength=0,domainsum = 0 ,spotdomain =0,iptr =0 ,intTempvalue=0;
	char *domainpartstring ,*domainstoreparth, *strTempDomain;
	icount = strlen(strDomain);
	domainpartstring =(char *)malloc(icount+100);
	domainstoreparth =(char *)malloc(icount+100);
	strTempDomain = (char *)malloc(strlen(strDomain)+10);
	strcpy(strTempDomain,strDomain);
    
	intTempvalue = strchr(strTempDomain,'.')-strTempDomain ;
	if (intTempvalue < 0)
	{
		return -1;
	}
	strncpy(domainpartstring,strTempDomain,intTempvalue);
	domainpartstring[intTempvalue] = '\0';
	if(strlen(domainpartstring)>3)
	{
		for(iptr = 0; iptr < strlen(domainpartstring); iptr ++)
		{
			if(domainpartstring[iptr]>=97 && domainpartstring[iptr]<=122)
			{
				domainsum += domainpartstring[iptr];
			}
			else
			{
				domainsum += 97;
			}
		}
		domainsum = domainsum/iptr;
	}
	else
	{
		domainsum = 97;
	}
	sprintf(domainstoreparth,"%s/%c/%c/%c/",DEFAULTROOT,domainpartstring[0],domainpartstring[strlen(domainpartstring)-1],domainsum);
	strcpy(strDomainPath,domainstoreparth);
	free(domainpartstring);
	free(strTempDomain);
	free(domainstoreparth);
	return 0;
}


int modiDomainNameFile(char *strDomainFile,char *strDomainConfigvalue)
{							//操作类型        域名           操作值            操作记录       操作记录类型      原来的值          mx类型
	FILE *f_date, *f_tmpfile;
	int intFound=0,i=0;
	char *strFilenametemp , *strConfigFile ,  *strFindstrings  ;
	char strTempLine[512];

	strConfigFile = (char *)malloc(strlen(strDomainFile) + 100);
	strFindstrings = (char *)malloc( 100);

	sprintf(strFindstrings,"$TTL    300\n");
	sprintf(strConfigFile,"%s",strDomainFile);
	strFilenametemp =(char *)malloc(strlen(strConfigFile) + 10 );

	sprintf(strFilenametemp,"%s.swf",strConfigFile);


//	printf("strFilenametemp1=%s\r\n",strFilenametemp);


	//strFilenametemp 是临时文件                 strConfigFile 是需要设置的文件

	if(lockp(strFilenametemp))
	{
		//printf("lock file %s\n",strFilenametemp);
		return  -1 ;  //临时文件已经被存取
	}//锁住文件
	f_date = fopen(strConfigFile, "r");
	if(f_date == NULL)
	{
		unlockp(strFilenametemp);
		return -1;//无法打开配置文件
	}
	f_tmpfile = fopen(strFilenametemp, "w");
	if(f_tmpfile == NULL)
	{
		fclose(f_date);
		unlockp(strFilenametemp);
		return -1;
	}
	//printf("strFindstrings=%s\r\n",strFindstrings);
	while(!feof(f_date))
	{
		bzero(strTempLine, sizeof(strTempLine));
		fgets(strTempLine, 511, f_date);
		if(!intFound)
		{
			//printf("strcmp(%s,%s)=%d\r\n",strTempLine,strFindstrings,!strcmp(strTempLine,strFindstrings));
			if(!strcmp(strTempLine,strFindstrings)) //找到添加的地方
			{
				intFound=1;
				for(i=0;i<8;i++ )
				{
					bzero(strTempLine, sizeof(strTempLine));
					fgets(strTempLine, 511, f_date);
					bzero(strTempLine, sizeof(strTempLine));
				}
				fprintf(f_tmpfile, "%s\n",strDomainConfigvalue);
			}
		}
		fputs(strTempLine,f_tmpfile);  //
	}

	fflush(f_tmpfile);
	fclose(f_tmpfile);
	fclose(f_date);
	rename(strFilenametemp,strConfigFile);
	chmod(strConfigFile,0664);
	free(strFindstrings);
	free(strDomainFile);
	free(strFilenametemp);
	free(strConfigFile);
	return 0;
}


/*char *addNewDomain(char *strDomain,int intTTL)
{
	uid_t uid;
	gid_t gid;

	char *strDomainSerial, *strDomainFilename,*strDomainpath , *strDomainConfigvalue , *strNamedConfigvalue;

	strDomainpath = (char *)malloc(100);
	strDomainConfigvalue = (char *)malloc(1024);
	strNamedConfigvalue = (char *)malloc(1024);
	strDomainFilename = (char *)malloc(100);
	strDomainSerial= (char *)malloc(20);
	getDomaindir(strDomainpath,strDomain);
	getDomainSerial(strDomainSerial);

	sprintf(strDomainFilename,"%s%s.name",strDomainpath,strDomain);
	sprintf(strNamedConfigvalue,"zone \"%s\" IN {\n        type master;\n        file \"%s%s.name\";\n        allow-update { 127.0.0.1; };\n};\n",strDomain,strDomainpath,strDomain);
	//sprintf(strDomainConfigvalue,"$TTL    300\n$ORIGIN %s.\n@       IN      SOA     %s. dnsconct.myhostadmin.net.  (\n                                      %s ; Serial\n                                      3600      ; Refresh\n                                      300      ; Retry\n                                      68400      ; Expire\n                                      %d   )    ; Minimum\n                IN      NS      %s.\n",strDomain,strDnsServerName,strDomainSerial,intTTL,strDnsServerName);
	sprintf(strDomainConfigvalue,"$TTL    500\n$ORIGIN %s.\n@       IN      SOA     %s. dnsconct.myhostadmin.net.  (\n\t\t\t\t%s ; Serial\n\t\t\t\t3600      ; Refresh\n\t\t\t\t300      ; Retry\n\t\t\t\t1209600      ; Expire\n\t\t\t\t%d   )    ; Minimum\n\t\t\t3600    NS      %s.\n\t\t\t3600    NS      %s.\n\t\t\t3600    NS      %s.\n\t\t\t3600    NS      %s.\n\t\t\t3600    NS      %s.\n\t\t\t3600    NS      %s.\n",strDomain,DNS_1,strDomainSerial,intTTL,DNS_1,DNS_2,DNS_3,DNS_4,DNS_5,DNS_6);

	if(!(addConfigFiles(strNamedConfigvalue,strDomain)==0))
	{
		return REGMSG_601; //1.写named.conf
	}
	//printf("strDomainFilename=%s\r\nstrDomainConfigvalue=%s\r\n",strDomainFilename,strDomainConfigvalue);
	if(!(createNewFile(strDomainFilename,strDomainConfigvalue)==0))
	{
		return REGMSG_602; 	//2.添加新文件 yuming.com.name
	}

	getUGID(&uid,&gid,"named");	
	if (uid >0 && gid >0)
	{
		chown(strDomainFilename,uid,gid);
		chown(DEFAULTCONFIG,uid,gid);
	}

	free(strDomainFilename);
	free(strNamedConfigvalue);
	free(strDomainConfigvalue);
	free(strDomainpath);
	return REGMSG_200;
}
*/

char *modiOldDomain(char *strDomain,char *strDnsServerName,int intTTL)
{
	char *strDomainSerial, *strDomainFilename,*strDomainpath , *strDomainConfigvalue  ;

	strDomainpath = (char *)malloc(100);
	strDomainConfigvalue = (char *)malloc(1024);
	strDomainFilename = (char *)malloc(100);
	strDomainSerial= (char *)malloc(20);
	getDomaindir(strDomainpath,strDomain);
	getDomainSerial(strDomainSerial);

	sprintf(strDomainFilename,"%s%s.name",strDomainpath,strDomain);
	sprintf(strDomainConfigvalue,"$TTL    300\n$ORIGIN %s.\n@       IN      SOA     %s. dnsconct.centerdns.com.  (\n                                      %s ; Serial\n                                      600      ; Refresh\n                                      300      ; Retry\n                                      68400      ; Expire\n                                      %d   )    ; Minimum\n                IN      NS      %s.\n",strDomain,strDnsServerName,strDomainSerial,intTTL,strDnsServerName);
	//printf("modiOldDomain\r\n");
	//printf("strDomainFilename=%s\r\n",strDomainFilename);
	//printf("strDomainConfigvalue=%s\r\n",strDomainConfigvalue);
	if(!(modiDomainNameFile(strDomainFilename,strDomainConfigvalue)==0))
	{
		return REGMSG_602; 	//2.添加新文件 yuming.com.name
	}
	free(strDomainFilename);
	free(strDomainConfigvalue);
	free(strDomainpath);
	free(strDomainSerial);
	return REGMSG_200;
}


/*char *delDomain(char *strDomain)
{
	char *strDomainFilename,*strDomainpath , *strDomainConfigvalue , *strNamedConfigvalue;
	strDomainpath =(char *)malloc(100);
	strDomainConfigvalue = (char *)malloc(1024);
	strNamedConfigvalue = (char *)malloc(1024);
	strDomainFilename = (char *)malloc(100);
	bzero(strDomainpath, sizeof(strDomainpath));
	bzero(strDomainConfigvalue, sizeof(strDomainConfigvalue));
	bzero(strNamedConfigvalue, sizeof(strNamedConfigvalue));
	bzero(strDomainFilename, sizeof(strDomainFilename));
	getDomaindir(strDomainpath,strDomain);
	sprintf(strDomainFilename,"%s%s.name",strDomainpath,strDomain);
	unlockp(strDomainFilename);
	if(!(delConfigFiles(strDomain)==0))
	{
		free(strDomainFilename);
		free(strNamedConfigvalue);
		free(strDomainConfigvalue);
		free(strDomainpath);
		return  REGMSG_603;
	}
	//1.写named.conf
	//2.添加新文件 yuming.comhost.zone
	free(strDomainFilename);
	free(strNamedConfigvalue);
	free(strDomainConfigvalue);
	free(strDomainpath);
	return REGMSG_200;
}*/

char *delDomain(NEWDOM* zone)
//返回null 有问题
{
	FILE *f_data,*f_tmpfile;  
    uid_t uid;
    gid_t gid;
    char  strConfigFile[25]={0};
    char  strFilenametemp[25]={0};
    char  strDomainpath[100]={0};
    char  strDomainFilename[100]={0};
	char  strDomainlogFilename[100]={0};
	char  strTempLine[512]={0};
	char instr[50]={0};
	char instrzone[100]={0};
    NEWDOM *p,*q;
	int str_tag=0;
	int i;
    
	if (!zone && !zone->next) return NULL;
	strcpy(strConfigFile,DEFAULTCONFIG);
	sprintf(strFilenametemp,"%s.swf",strConfigFile);
    f_data=f_tmpfile=NULL;
	f_data=fopen(strConfigFile,"r");
	f_tmpfile=fopen(strFilenametemp,"w");
	sprintf(instr,"zone \"");
    
	if (!f_data) return NULL;
    
    // while(!feof(f_data)){
       //  fgets(strTempLine,512,f_data);
	 while(fgets(strTempLine,512,f_data)!=NULL){
		 str_tag=0;
         if (strstr(strTempLine,instr))
         {
			 p=zone->next;
			 while (p)
              {
				 sprintf(instrzone,"\"%s\"",p->mtext);
	             if (strstr(strTempLine,instrzone))
	            {
					 str_tag=1;
                     for(i=0;i<5;i++ )
                         {
                             bzero(strTempLine, sizeof(strTempLine));
                             fgets(strTempLine, 511, f_data);							 
                         }
				    break;
	            }
				p=p->next;
		    }//endwhilep
              
         }
		 
      if (!str_tag) fputs(strTempLine,f_tmpfile);
      }//endwhilefeof


      fflush(f_tmpfile);
	  fclose(f_tmpfile);
	  fclose(f_data);
      rename(strFilenametemp,strConfigFile);
      getUGID(&uid,&gid,"named");
      if (uid >0 && gid >0)
	   {
		    chown(strDomainFilename,uid,gid);
		    chown(DEFAULTCONFIG,uid,gid);
	   }



      p=zone->next;
	  while(p) {
	     getDomaindir(strDomainpath,p->mtext);
	     sprintf(strDomainFilename,"%s%s.name",strDomainpath,p->mtext);
	     sprintf(strDomainlogFilename,"%s%s.name.jnl",strDomainpath,p->mtext);
	     unlink(strDomainFilename);
		 unlink(strDomainlogFilename);
         q=p;
         p=q->next;
		 free(q);
	  }
	  zone->next=NULL;
	return "del domain seccuess.\n";
}


int createNewFile(char *strFilename ,char *strFilevalue)
{
	FILE	*f_data;
    uid_t uid;
    gid_t gid;
	//if(lockp(strFilename))
	//{
	//	return -1;
	//}
	//else
	//{
		f_data = fopen(strFilename, "w");
	//}

	fprintf(f_data, "%s\n",strFilevalue);
	fflush(f_data);
	fclose(f_data);
	//chown(,25,25);
	chmod(strFilename,0664);
	getUGID(&uid,&gid,"named");
    if (uid >0 && gid >0)
	  {
		  chown(strFilename,uid,gid);
	  }
	return 0;
}


/*int delConfigFiles(char *strDomain )
{
        FILE *f_date, *f_tmpfile;
        int intFound=0 , i;
        char *strFilenametemp , *strConfigFile ,*strConfigstring ;
        char strTempLine[512];

        strConfigstring = (char *)malloc(strlen(strDomain)+50);
        sprintf(strConfigstring,"zone \"%s\" IN {",strDomain);
    strConfigFile = (char *)malloc(strlen(DEFAULTCONFIG) + 10 );
        bzero(strConfigFile, sizeof(strConfigFile));
        strcpy(strConfigFile,DEFAULTCONFIG);
        strFilenametemp =(char *)malloc(strlen(strConfigFile) + 10 );
        bzero(strFilenametemp, sizeof(strFilenametemp));

        sprintf(strFilenametemp,"%s.swf",strConfigFile);

        if(lockp(strFilenametemp))
        {
                printf("lock file %s\n",strFilenametemp);
                return -1;
        }

        f_date = fopen(strConfigFile, "r");
        if(f_date == NULL)
        {
                unlockp(strFilenametemp);
                return -2;
        }
        f_tmpfile = fopen(strFilenametemp, "w");
        if(f_tmpfile == NULL)
        {
                fclose(f_date);
                unlockp(strFilenametemp);
                return -2;
        }

        while(!feof(f_date))
        {
                bzero(strTempLine, sizeof(strTempLine));
                fgets(strTempLine, 511, f_date);
                if(!intFound)
                {
                        if(strCASEstr(strTempLine, strConfigstring))
                        {
                                intFound=1;
                                for(i=0;i<5;i++ )
                                {
                                        bzero(strTempLine, sizeof(strTempLine));
                                        fgets(strTempLine, 511, f_date);
                                }
                        }
                }
                fputs(strTempLine,f_tmpfile);  //
        }

        fflush(f_tmpfile);
        fclose(f_tmpfile);
        fclose(f_date);
        rename(strFilenametemp,strConfigFile);
        chown(strConfigFile,25,25);

        unlockp(strFilenametemp);
        free(strConfigstring);
        free(strFilenametemp);
        free(strConfigFile);
        return 0;
}
*/

/*int addConfigFiles(char *strNewvalue , char *strDomain)
{
        FILE *f_date, *f_tmpfile;
        int intFound=0;
        char *strFilenametemp , *strConfigFile;
        char *firstLinetepvalue ;
        char strTempLine[512];
        firstLinetepvalue= (char *)malloc(strlen(strDomain) +10 );
        strConfigFile = (char *)malloc(strlen(DEFAULTCONFIG) + 10 );
        bzero(strConfigFile, sizeof(strConfigFile));
        strcpy(strConfigFile,DEFAULTCONFIG);
        strFilenametemp =(char *)malloc(strlen(strConfigFile) + 10 );
        bzero(strFilenametemp, sizeof(strFilenametemp));
        sprintf(strFilenametemp,"%s.swf",strConfigFile);
        sprintf(firstLinetepvalue,"\"%s\"",strDomain);
        if(lockp(strFilenametemp))
        {
                printf("lock file %s\n",strFilenametemp);
                return -1;
        }
        f_date = fopen(strConfigFile, "r");
        if(f_date == NULL)
        {
                unlockp(strFilenametemp);
                return -2;
        }
        f_tmpfile = fopen(strFilenametemp, "w");
        if(f_tmpfile == NULL)
        {
                fclose(f_date);
                unlockp(strFilenametemp);
                return -2;
        }
        while(!feof(f_date))
        {
                bzero(strTempLine, sizeof(strTempLine));
                fgets(strTempLine, 511, f_date);
                if(strCASEstr(strTempLine, firstLinetepvalue)!=NULL)
                {
                        printf("now find the %s in %s", firstLinetepvalue , strTempLine);
                        intFound=-1;
                        break;
                }
                if(!intFound)
                {
                        if(strCASEstr(strTempLine, DEFAULTREPLACEVALUE ))
                        {
                                bzero(strTempLine, sizeof(strTempLine));
                                intFound=1;
                                fprintf(f_tmpfile,"%s\n%s\n",DEFAULTREPLACEVALUE , strNewvalue);
                        }
                }
                fputs(strTempLine,f_tmpfile);//  firstLinetepvalue
        }
        if(intFound==-1)
        {
                fflush(f_tmpfile);
                fclose(f_tmpfile);
                fclose(f_date);
                unlockp(strFilenametemp);
                unlink(strFilenametemp);
                free(strFilenametemp);
                free(strConfigFile);
                return -1;
        }
        fflush(f_tmpfile);
        fclose(f_tmpfile);
        fclose(f_date);
        rename(strFilenametemp,strConfigFile);
        chown(strConfigFile,25,25);
        unlockp(strFilenametemp);
        free(strFilenametemp);
        free(strConfigFile);
        return 0;
}*/


//-===============================================================add new func here
/*
 *   转发指令
 */
void sndCmd(cmdlog *cmdlog_lst,serverlist *seriplst,msgbuf *gbuf){
     serverlist *p;
	 p=seriplst->nextip;
	 char iplst[16]={0};
	 char s_msg[MAXMSGSIZE]={0};
	 char md5key[33]={0};
	 char s_act[4]={0};
     int i=0;

     while (p)
    {
	   strcpy(iplst,p->serip);
	   if (!p->tag)
	   {
		 
		 if (gbuf->act==0)
		 {
             strcpy(s_act,"del");
		 }else {
		     strcpy(s_act,"add");
		 }
		 sprintf(s_msg,"%s\r\nentityname:dnsdomain\r\ndomainname:%s\r\ndnsservername:ns1.myhostadmin.net\r\n",s_act,gbuf->mtext);
		 mk_md5(s_msg,md5key);
         sprintf(s_msg,"%s\r\nentityname:dnsdomain\r\ndomainname:%s\r\ndnsservername:ns1.myhostadmin.net\r\nkeymd5:%s\r\n.\r\n",s_act,gbuf->mtext,md5key);
		 		 
		 int result=0;
		 do{	 
				 result=sndMsg(s_msg,iplst,7890);
			     if (result>0) break;
				 i++;
		 }while(i<3) ; //send three time.
               
          if (result<0)
          {   
			  printf("add to list!");
			  p->tag=1;                         //send mess to server failure
			  if (addCmdlog(cmdlog_lst,gbuf,iplst)<0)   //log err.
			  {
				perror("sndCmd:writecmdlog err.");
			  }
          }
		 
	    }else {
		   if (addCmdlog(cmdlog_lst,gbuf,iplst)<0)     //log err.
			  {
				perror("sndCmd:writecmdlog err.");
			  }
		}//endifp->tag
		 p=p->nextip;
		 i=0;
		 bzero(md5key,33);
		 bzero(s_msg,MAXMSGSIZE);
		 bzero(iplst,16);
		 bzero(s_act,4);
     }//endwhile
	 return;
}
/*   发送消息到服务器
 *   sndmsg:消息内容
 */
int sndMsg(char *sndmsg,char *serverip,int serverport) {  
      int len,socket_fd;  
	  int result,i;
	  int interti,ti;       //依次是时间间隔和发送次数

      socket_fd=openClientSocket(serverip,serverport);
         if (!socket_fd||socket_fd<0){
               perror("connection failed.");
		       return -1;
		 }
      result=send(socket_fd,sndmsg,strlen(sndmsg),0);
	       if (result<0)
	          {
		       perror("send msg err.");
			   close(socket_fd);
			   return -1;
	          }
	  close(socket_fd);
	  return 1;
}

/*
static int checkzonef(char *zone,char *zonepathfile,FILE *f_conf){
//返回0 全加,返回1,加区文件,返回-1,不加,返回2,只加配置
FILE *fcmd;
char strcheckzone[512]={0};
char digbuf[2048]={0};
char confbuf[2048]={0};
char zonename[512]={0};
char *tag=NULL;
struct stat  *zonefilestat;
int result;
sprintf(strcheckzone,"dig @127.0.0.1 %s",zone);

sprintf(zonename,"\"%s\"",zone);
fcmd=popen(strcheckzone,"r");
if (fcmd) {
   while(!feof(fcmd)){
      fgets(digbuf,sizeof(digbuf),fcmd);
      tag=strstr(digbuf,"myhostadmin.net");          //已经加载
      if (tag) {
		  pclose(fcmd);
          return -1;
      }elseif (tag=strstr(digbuf,"status:SERVFAIL"))
	    {                                             //需要添加区文件
		  pclose(fcmd);
          return 1;
        }
	}//endwhilefcmd
}		  
pclose(fcmd);
if(!stat(zonepathfile,zonefilestat)){             //区文件是否存在	  
    while(!feof(f_conf)){
     fgets(confbuf,2048,f_conf);
     if (strstr(confbuf,zonename)){                     //域名配置是否存在
        return -1;
     }else{
        return 2;
     }
    }//endwhilef_conf
  }
 return 0;
}
*/
NEWDOM* linked(NEWDOM* root,msgbuf *addmsg){
  NEWDOM *p,*tail,*q;
  int i;                                           //资源计数器
  tail=root;
  q=p=NULL;
  char strzone[512]={0};
  char strlink[512]={0};

  if (root->tag>=0) i=root->tag;                  //当前表中的资源个数
  while (tail)
     { 
	   sprintf(strzone,"\"%s\"",addmsg->mtext);
	   sprintf(strlink,"\"%s\"",tail->mtext);
	   if (strstr(strlink,strzone)) return NULL; 
		  q=tail;
          tail=tail->next; 
	   bzero(strzone,sizeof(strzone));
	   bzero(strlink,sizeof(strlink));
	 }    
   p = (NEWDOM *)malloc(sizeof(NEWDOM));
   if (!p)  return NULL;
   p->next=NULL;
   p->tag=0;
   strcpy(p->mtext,addmsg->mtext);
   q->next=p;
   i++;
   root->tag=i;
   return p;
}

int checkzone(NEWDOM* zone){
//0 正常 -1有误
FILE* f_nconf;
char  strConfigFile[25]={0};
char  strconfbuf[512]={0};
char  strzone[512]={0};
char  instr[50]={0};
int   i=0;                                              //当前需要检测的资源个数
NEWDOM *p;

p=zone->next;
if (NULL==p) return -1;
if ((i=zone->tag)==0) return -1;                      
strcpy(strConfigFile,DEFAULTCONFIG);
sprintf(instr,"zone \"");
f_nconf=fopen(strConfigFile,"r");
if (NULL==f_nconf) return -1;
                                      
//while(!feof(f_nconf)){
 //     fgets(strconfbuf,512,f_nconf);
while(fgets(strconfbuf,512,f_nconf)!=NULL) {
	  if (i==0) break;
      if (strstr(strconfbuf,instr)){
		  p=zone->next;
         for(;p;)
             {
			   //当前需要检测的资源个数为0,不需在继续检查,直接跳出
			   if (i==0) break;
			  /* if (p->tag==1) {
				   p=p->next;
				   bzero(strzone,sizeof(strzone));
				   continue;
			   }*/
			   
			   if (p->tag==0)
			   {
				   sprintf(strzone,"\"%s\"",p->mtext);
                   if (strstr(strconfbuf,strzone)){				   
                    p->tag=1;
					i--;
					//break;
                   }  
			   }
               p=p->next;
			   bzero(strzone,sizeof(strzone));
             }//endforp
        }
  bzero(strconfbuf,sizeof(strconfbuf));
  }//endfeof
  fclose(f_nconf);
  zone->tag=i;
  return 0;
}

char *addDomain(NEWDOM * dom,int tag){
   //批量添加域名，添加到两个配置文件，一个区文件
    uid_t uid;
    gid_t gid;
   
    FILE *f_date,*f_tmpfile;                                                                                  //named.conf
	FILE *f_update;                                                                                           //lwresd.conf

     
    char  strDomainConfigvalue[1024]={0};
	char  strNamedConfigvalue[1024]={0};
    char  strConfigFile[25]={0};
    char  strUpdateFile[25]={0};
    char  strFilenametemp[25]={0};
	char  strUpdatenametemp[25]={0};
    char  strDomainpath[100]={0};
    char  strDomainFilename[100]={0};
    char  strDomainSerial[20]={0};
	char  strTempLine[512]={0};                                                                               //读取文件缓存区512字节
  	int   addopt,opttag;                                                                                      //addopt添加域名的选项，opttag配置文件选项标记
	int   i;                                                                                                  //资源个数计数
     
	addopt=opttag=0;                                                                                  
	
    int intFound=0;                                                                                           //添加域名文件标记
    char *firstLinetepvalue;                                                                                  //in for
	NEWDOM *p,*q;
    //如果当前需要添加的域名个数为0,则直接退出
	if (0==(i=dom->tag)) 
	{                                                                                   
	    q=dom->next;	
		while(q){
		  p=q;
		  q=p->next;                                                                      
		  free(p);
	      }//endwhile
	 dom->next=NULL;
     return "no domain to add.\n";
	 }

	f_date=f_tmpfile=f_update=NULL;
    //sprintf(firstLinetepvalue,"\"%s\"",strDomain);                                                          //in for

	strcpy(strConfigFile,DEFAULTCONFIG);
	strcpy(strUpdateFile,UPDATECONFIG);
   
    //strFilenametemp=(char *)malloc(strlen(strConfigFile)+10);                                                 //临时配置文件初始化
	//strUpdatenametemp=(char *)malloc(strlen(strUpdateFile)+10);
    sprintf(strFilenametemp,"%s.swf",strConfigFile);
	sprintf(strUpdatenametemp,"%s.swf",strUpdateFile);

	//strDomainpath = (char *)malloc(100);                                                                     //配置文件参数，for循环用
	//strDomainConfigvalue = (char *)malloc(1024);                                                             //区文件用配置
	//strNamedConfigvalue = (char *)malloc(1024);                                                              //named.conf文件用配置
	//strDomainFilename = (char *)malloc(100);                                                                 //区文件路径
	//strDomainSerial= (char *)malloc(20);                                                                     //随机生成的序列号
    
	f_date=fopen(strConfigFile,"r");                                                                         //打开named配置文件
	f_tmpfile=fopen(strFilenametemp,"w");                                                                    //打开named临时文件

	f_update=fopen(strUpdatenametemp,"w");                                                                   //打开lwresd.conf临时文件


   
    q=dom->next;
	//while(!feof(f_date)){
	  //     fgets(strTempLine,512,f_date);
    while(fgets(strTempLine,512,f_date)!=NULL){
           if (!strCASEstr(strTempLine,DEFAULTREPLACEOPT))                                                   //读配置文件选项
           {
			   if (!opttag)
			   {
				   fputs(strTempLine,f_update);
			   }
			   
           }else{ 
			   opttag = 1;
			}
           if (strCASEstr(strTempLine,DEFAULTREPLACEVALUE))
		       {
                          intFound=1;
                          fputs(strTempLine,f_tmpfile);
					      for (;q; )
						  {
							 
                             if (!getDomaindir(strDomainpath,q->mtext))
						        {
	                              getDomainSerial(strDomainSerial);
                                  sprintf(strDomainFilename,"%s%s.name",strDomainpath,q->mtext);                 //格式化配置文件内容和区文件内容                                 
								  if(!q->tag) {
									    if (tag)
								         {
									       sprintf(strNamedConfigvalue,"zone \"%s\" IN {\n        type master;\n        file \"%s%s.name\";\n        allow-update { 127.0.0.1; };\n};\n",q->mtext,strDomainpath,q->mtext);
								         }else {
                                           sprintf(strNamedConfigvalue,"zone \"%s\" IN {\n        type slave;\n         file \"%s%s.name\";\n       masters { 61.139.126.202; };\n};\n",q->mtext,strDomainpath,q->mtext);
								         }
								           sprintf(strDomainConfigvalue,"$TTL    500\n$ORIGIN %s.\n@       IN      SOA     %s. dnsconct.myhostadmin.net.  (\n\t\t\t\t%s ; Serial\n\t\t\t\t3600      ; Refresh\n\t\t\t\t300      ; Retry\n\t\t\t\t1209600      ; Expire\n\t\t\t\t%d   )    ; Minimum\n\t\t\t3600    NS      %s.\n\t\t\t3600    NS      %s.\n\t\t\t3600    NS      %s.\n\t\t\t3600    NS      %s.\n\t\t\t3600    NS      %s.\n\t\t\t3600    NS      %s.\n",q->mtext,DNS_1,strDomainSerial,3600,DNS_1,DNS_2,DNS_3,DNS_4,DNS_5,DNS_6);
                             
							             fprintf(f_tmpfile,"%s\n",strNamedConfigvalue);          //写入配置内容
                                         fprintf(f_update,"%s\n",strNamedConfigvalue);                                   //写入更新配置文件
                                         createNewFile(strDomainFilename,strDomainConfigvalue);                          //添加区文件 			
								      }
							    }
                             p=q;
							 q=p->next;                                                                      //清理内存节点
							 free(p);
							 i--;
                                      
							 bzero(strDomainpath,sizeof(strDomainpath));                                     //清理缓存数据
							 bzero(strDomainSerial,sizeof(strDomainSerial));
							 bzero(strDomainFilename,sizeof(strDomainFilename));
                             bzero(strNamedConfigvalue,sizeof(strNamedConfigvalue));
							 bzero(strDomainConfigvalue,sizeof(strDomainConfigvalue));
                             
                         }//end for  数据写完
						 
						 dom->next=NULL;
						 dom->tag=0;
						 bzero(strTempLine,sizeof(strTempLine));
						 continue;
			   } 
			   fputs(strTempLine,f_tmpfile);                                                                 //配置文件复制原始数据
               bzero(strTempLine,sizeof(strTempLine));
	}//endwhile
	           
			   fflush(f_tmpfile);                                                                            //清理缓存准备关闭文件			   
			   fclose(f_tmpfile);			   
			   fclose(f_date);
			   fflush(f_update);
			   fclose(f_update);
			   f_update=NULL;
			   f_date=NULL;
			   f_tmpfile=NULL;
			   rename(strFilenametemp,strConfigFile);
			   rename(strUpdatenametemp,strUpdateFile);
			   getUGID(&uid,&gid,"named");
               if (uid >0 && gid >0)
	              {
		             chown(strDomainFilename,uid,gid);
		             chown(DEFAULTCONFIG,uid,gid);
			    	 chown(strUpdateFile,uid,gid);
	              }
               system("rndc addzone");                                                                        //立即加载新域名
			  
			   return REGMSG_200;
} 


char *linux_dns(char * execute_buf)
{
    int result;
	#define valid_com_num_dns 7
	int	i_valid_com, i_temp_valid_com = 0, i_valid_com_len,intTTLvalue , intMxlevel ,intRRTTL, intActionType;
	char *StrTmpvalue;
	char *StrValidCom;
	char *strExecStrings;
	char *return_result;
	char *strDomain,*strResValue,*strResHost,*strResType,*strResOldValue,*strDnsServer;
	char * p_temp,*p_domainptr;
	return_result=(char *)malloc(512);
    msgbuf smsg;
	int i = 0;
   
	char *valid_dom[valid_com_num_dns] =
	{
		"add\r\nentityname:dnsdomain\r\n",    //0
		"check\r\nentityname:dnsdomain\r\n",   //暂时没用的
		"add\r\nentityname:dnsrecord\r\n",
		"mod\r\nentityname:dnsrecord\r\n",
		"del\r\nentityname:dnsrecord\r\n",
		"del\r\nentityname:dnsdomain\r\n",    //5
		"mod\r\nentityname:dnsdomain\r\n"     //6
		"reload\r\nentityname:dnsdomain\r\n"   //不常用

	};
	for(i_valid_com = 0; i_valid_com < valid_com_num_dns; i_valid_com ++)
	{
		StrValidCom = valid_dom[i_valid_com];
		i_valid_com_len = strlen(StrValidCom);
		if(!strncasecmp(execute_buf, StrValidCom,i_valid_com_len))
		{
			i_temp_valid_com = 1;
			break;
		}
	}
	strDomain = (char *)malloc(512);
	if(!(getvalueCheckType(execute_buf,"domainname", ":",strDomain, "\r\n","string")==0))
	{
		strcpy(return_result, REGMSG_421);
		return return_result;
	}
	trim(strDomain, 3);

    //==
	p_domainptr=strDomain;
	
	if (strlen(strDomain) > MAXMSGSIZE)
	{
         strcpy(return_result, "domainname too long.");
		 return return_result;
	}
	for (;i <strlen(strDomain) ; )
	{
	  if ( 44 > *p_domainptr > 0 || *p_domainptr > 122 )
	  {
           sprintf(return_result, "invalid domain name %s.\n",strDomain);
		   return return_result;
	  }
	  p_domainptr++;
	  i++;
	}//endfor
	    //sprintf(return_result, "domian %s is good,ptr is %s.\n",strDomain,p_domainptr);
        //return return_result;

	switch(i_valid_com)
	{
		case 0:			//add dns domain
			StrTmpvalue = (char *)malloc(512);
			if(!(getvalueCheckType(execute_buf,"ttl", ":",StrTmpvalue, "\r\n","int")==0))
			{
				sprintf(StrTmpvalue,"%s",DEFAULTTTL);
			}
			intTTLvalue = atoi(StrTmpvalue);
			strDnsServer = (char *)malloc(512);
//-=============发送消息
            bzero((char *)&smsg,sizeof(smsg));
			smsg.mtype=1;
			sprintf(smsg.mtext,strDomain);
			smsg.ttl=intTTLvalue;
			smsg.act=1;
			result=msgsnd(qid,&smsg,sizeof(smsg)-sizeof(long),IPC_NOWAIT);
            if (result<0)
            {
				strcpy(return_result, "msgSnd:domain add %s err.\r\n");
            }else if (result==0)
            {
                  strcpy(return_result,REGMSG_200);
            }
			//p_temp =addNewDomain(strDomain,intTTLvalue);
			//strcpy(return_result,p_temp);
			intActionType = -1;
			free(StrTmpvalue);
			free(strDnsServer);
			break;
		case 1:			//check dns domain exists
			strcpy(return_result,REGMSG_700);
			intActionType = -1;
			break;
		case 2:			//add dns record
			intActionType=0;
			break;
		case 3:			//mod dns record
			intActionType=1;
			break;
		case 4:			//del dns record
			intActionType=2;
			break;
		case 5:			//del dns domain
//-===============发送消息
            bzero((char *)&smsg,sizeof(smsg));
			smsg.mtype=1;
			smsg.act=0;
			smsg.ttl=0;
			sprintf(smsg.mtext,strDomain);
			result=msgsnd(qid,&smsg,sizeof(smsg)-sizeof(long),IPC_NOWAIT);
            if (result<0)
            {
				strcpy(return_result, "msgSnd:del domain err.\r\n");
            }else if (result==0)
            {
                  strcpy(return_result,REGMSG_200);
            }
			//p_temp = delDomain(strDomain);
			//strcpy(return_result,p_temp);
			intActionType=-1;
			break;
		case 6:			//refresh dns domain
			strcpy(return_result,REGMSG_500);
			intActionType = -1;
			break;
		case 7:
			strExecStrings = (char *)malloc(512);
			sprintf(strExecStrings,"/usr/sbin/rndc reload %s",strDomain);
			if(!(execbaiumailcmd(strExecStrings)==0))
			{
				strcpy(return_result, REGMSG_540);
			}
			strcpy(return_result,REGMSG_200A);
			intActionType = -1;
			break;

		default: //找不到命令
			intActionType=-1;
			strcpy(return_result, REGMSG_500);
	}
	if(!(intActionType==-1))
	{

		strResType = (char *)malloc(512);
		StrTmpvalue = (char *)malloc(512);
		bzero(StrTmpvalue, sizeof(StrTmpvalue));
		bzero(strResType,sizeof(strResType));
		if(!(getvalueCheckType(execute_buf,"resolvetype", ":",StrTmpvalue, "\r\n","string")==0))
		{
			strcpy(return_result, REGMSG_421);
			return return_result;
		}
		trim(StrTmpvalue, 3);
		myStrupr(strResType,StrTmpvalue);

		strResHost = (char *)malloc(512);
		if(!(getvalueCheckType(execute_buf,"resolvehost", ":",strResHost, "\r\n","string")==0))
		{
			strcpy(return_result, REGMSG_421);
			return return_result;
		}
		trim(strResHost, 3);


		strResValue = (char *)malloc(512);
		if(!(getvalueCheckType(execute_buf,"resolvevalue", ":",strResValue, "\r\n","string")==0))
		{
			strcpy(return_result, REGMSG_421);
			return return_result;
		}
		trim(strResValue, 3);


		strResOldValue=(char *)malloc(512);
		if(!(getvalueCheckType(execute_buf,"resolveoldvalue", ":",strResOldValue, "\r\n","string")==0))
		{
			free(strResOldValue);
			strResOldValue=NULL;
		}else
			trim(strResOldValue, 3);
		intMxlevel=10;
		if (!strcmp(strResType,DEFAULTMXTYPE))
		{
			bzero(StrTmpvalue, sizeof(StrTmpvalue));
			if(!(getvalueCheckType(execute_buf,"mxlevel", ":",StrTmpvalue, "\r\n","int")==0))
			{
				strcpy(return_result, REGMSG_421);
				return return_result;
			}
			intMxlevel  = atoi(StrTmpvalue);
		}
		
		bzero(StrTmpvalue,sizeof(StrTmpvalue));
		if (!(getvalueCheckType(execute_buf,"resolvettl",":",StrTmpvalue,"\r\n","int")==0))
			intRRTTL=900;
		else
			intRRTTL=atoi(StrTmpvalue);

		//开始解析

		p_temp=doUpdate(strDomain,intActionType,strResType,strResHost,strResValue,strResOldValue,intMxlevel,intRRTTL);
		strcpy(return_result,p_temp);

    
	free(strResType);
	free(StrTmpvalue);
    free(strResHost);
	free(strResValue);
	free(strResOldValue);
	}
	free(strDomain);
	return return_result;
}
//2013/7/15 15:36 
