#include "server.h"
#define TIMEOUT 1



void settimers(struct timespec *t){
    //定时1秒无反应即返回
    t->tv_sec=TIMEOUT;
	t->tv_nsec=0;
}

int setsocket(int socket_fd){
    int F_tag=0;
	int ret=0;
	struct timespec settimer;
    F_tag=fcntl(socket_fd,F_GETFL);
    if (fcntl(socket_fd,F_SETFL,F_tag|O_NONBLOCK))
	                   perror("set nonblocking err."),ret=-1;
    settimers(&settimer);
	if (setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&settimer,sizeof(settimer))<0)
	{
        ret=-1;
	}
    return ret;
}

int selectfunc(int socket_fd){
    fd_set fd_w;
    int ret=0;  
	struct timespec settimer;
	settimers(&settimer);
    FD_ZERO(&fd_w);
	FD_SET(socket_fd,&fd_w);
	if ((ret=select(socket_fd+1,NULL,&fd_w,NULL,&settimer))>0) {
		return(socket_fd);
	}else {
		FD_CLR(socket_fd,&fd_w);
                close(socket_fd);
                return 0;
	}
}

int myStrupr(char *strTempstr,char *strDostring )
{
        int intStringlength ,inti;
        intStringlength=strlen(strDostring);
                bzero(strTempstr, sizeof(strTempstr));
                sprintf(strTempstr,strDostring);
                //printf("strTempstr[0]=%c\r\n",strTempstr[0]);
        for (inti=0;inti<intStringlength;inti++)
        {
                if(strTempstr[inti]>=97 && strTempstr[inti]<=122)
                {
                     //printf("strTempstr[%d]=%c\r\n",inti,strTempstr[inti]);
                     strTempstr[inti] = strTempstr[inti] -32;
                }
        }
                strTempstr[intStringlength]='\0';
        return 0;
}

 
int trim(char *str, int mode)
{
	int i,	len, trimmed = 0;
	char *c;

	if (str==NULL) {
		//str=malloc(10);
		//str[0]='\0';	//  如果原来是NULL, 最好也返回空窜, 是否正确 2000-05-19
		return 0;
	}
	if (!*str){
		return 0;
	}
	len = strlen(str);
	c = (char *)malloc(sizeof(char) * (len + 1));;
	if (c == NULL) {
		return -1;
	}
	strcpy(c, str);
	if (mode & 1) {
		for (i = 0; i < len; i++) {
			if (c[i] == ' ' || c[i] == '\n' || c[i] == '\r' || c[i] == '\t' || c[i] == '\v') trimmed++;
			else break;
		}
		len -= trimmed;
		c += trimmed;
	}
	if (mode & 2) {
		for (i = len - 1; i >= 0; i--) {
			if (c[i] == ' ' || c[i] == '\n' || c[i] == '\r' || c[i] == '\t' || c[i] == '\v') len--;
			else break;
		}
	}
	c[len]='\0';
	strcpy(str,c); str[len]='\0';
	free(c);
	return 0;
} // end of trim()

int openServerSocket(int intListenPort)
{
	struct	sockaddr_in srvaddr;
	int socket_fd;
	
	if ((socket_fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		return -1;
	}

	bzero(&srvaddr, sizeof(srvaddr));
	srvaddr.sin_family=AF_INET;
	srvaddr.sin_port=htons(intListenPort);
	srvaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	//srvaddr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);

	if(bind(socket_fd, (struct sockaddr *)&srvaddr, sizeof(struct sockaddr))==-1)
	{
		return -2;
	}

	if(listen(socket_fd, 5) == -1)
	{
		return -3;
	}

	return socket_fd;
}

int openClientSocket(char *Server_addr, int Server_port)
{
	struct sockaddr_in sa;
	struct hostent *hostdata;
	int len, socket_fd;
	char *Server_ip;
    int ret=0;
	hostdata = gethostbyname(Server_addr);
	if(hostdata == NULL)
		Server_ip = Server_addr;
	else
		Server_ip = (inet_ntoa(*((struct in_addr*)(hostdata->h_addr_list[0]))));

	if ((socket_fd = socket (AF_INET, SOCK_STREAM, 0))==-1)	return(-5);
	if (setsocket(socket_fd)<0) return (-4);

	memset (&sa, '\0', sizeof(sa));
	sa.sin_family      = AF_INET;
	sa.sin_addr.s_addr = inet_addr (Server_ip); 
	sa.sin_port        = htons     (Server_port);

	if ((ret=connect(socket_fd, (struct sockaddr*) &sa, sizeof(sa)))==-1){
		 if (ret==0) return socket_fd;
	     if (EINPROGRESS==errno)
		 {
		         return selectfunc(socket_fd);
		 }else {
				 close(socket_fd);
	             return(-6);
		 }
	}
	return (-7);
}

char *strCASEstr(char *haystack, char *needle)
{
	char *strTemp;
	int indllen = 0, inthaylen = 0;
	if(haystack == NULL || needle == NULL)
	{
		return NULL;
	}

	indllen = strlen(needle);
	inthaylen = strlen(haystack);
	strTemp = haystack;
	if(indllen == 0)
	{
		return strTemp;
	}

	while(strTemp + indllen <= haystack + inthaylen)
	{
		if(!strncasecmp(strTemp, needle, indllen))
		{
			return strTemp;
		}
		strTemp ++;
	}
	return NULL;
}

int getvalue(char *StrSource, char *StrName, char *Strequal, char *StrValue, char *StrSeparator)
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

	return 0;
}

int checkAccessHost(char *flHostAcc, char *remoteIP)
{
	//检查一个IP是否在允许连接的IP上, 如果是返回1, 否返回0;
	char strLineBuffer[100], *ptrTemp;
	char *Accept_ip;
	struct hostent *hostdata;
	FILE *fHostAcc;
	
	if((fHostAcc = fopen(flHostAcc, "r")) == NULL)
	{
		return 0;
	}

	//fseek(fHostAcc, 0, SEEK_SET);
	while(!feof(fHostAcc))
	{
		strLineBuffer[0] = 0;
		fgets(strLineBuffer, 99, fHostAcc);
		ptrTemp = strchr(strLineBuffer, '#');
		if(ptrTemp != NULL)
		{
			*ptrTemp = 0;
		}
		trim(strLineBuffer, 3);
		if(!*strLineBuffer)
		{
			continue;
		}

		hostdata = gethostbyname(strLineBuffer);
		if(hostdata == NULL)
			Accept_ip = strLineBuffer;
		else
			Accept_ip = (inet_ntoa(*((struct in_addr*)(hostdata->h_addr_list[0]))));

		if(!strncmp(Accept_ip, remoteIP, 15))
		{
			fclose(fHostAcc);
			return 1;
		}
	}
	fclose(fHostAcc);
	return 0;
}

char **getMultiValues(int *num, char *StrSource, char *StrName, char *Strequal, char *StrSeparator)
{
	char **strValues;
	int intCount = 0;
	char *StrTemp, *StrTemp1, *StrTemp2, *strValue;

	StrTemp = strCASEstr(StrSource, StrName);
	while(StrTemp != NULL)
	{
		StrTemp += strlen(StrName);
		while(*StrTemp == ' ' || *StrTemp == '\t')
		{
			StrTemp ++;
		}
		StrTemp1 = StrTemp;
		if(strncmp(StrTemp, Strequal, strlen(Strequal)))
		{
			StrTemp2 = strstr(StrTemp1, StrSeparator);
			if(StrTemp2 == NULL)
			{
				break;
			}
			StrTemp2 += strlen(StrSeparator);
			StrTemp = strCASEstr(StrTemp2, StrName);
			continue;
		}
		
		StrTemp1 += strlen(Strequal);
		StrTemp2 = strstr(StrTemp1, StrSeparator);
		if(StrTemp2 == NULL)
		{
			break;
		}
		if(StrTemp2 == StrTemp1)
		{
			continue;
		}
		StrTemp2 += strlen(StrSeparator);
		StrTemp = strCASEstr(StrTemp2, StrName);
		intCount ++;
	}
	
	*num = intCount;
	if(intCount == 0)
	{
		return NULL;
	}

	strValues = (char **)malloc(sizeof(char *) * (intCount + 1));

	intCount = 0;
	StrTemp = strCASEstr(StrSource, StrName);
	while(StrTemp != NULL)
	{
		StrTemp += strlen(StrName);
		while(*StrTemp == ' ' || *StrTemp == '\t')
		{
			StrTemp ++;
		}
		StrTemp1 = StrTemp;
		if(strncmp(StrTemp, Strequal, strlen(Strequal)))
		{
			StrTemp2 = strstr(StrTemp1, StrSeparator);
			if(StrTemp2 == NULL)
			{
				break;
			}
			StrTemp2 += strlen(StrSeparator);
			StrTemp = strCASEstr(StrTemp2, StrName);
			continue;
		}
		
		StrTemp1 += strlen(Strequal);
		StrTemp2 = strstr(StrTemp1, StrSeparator);
		if(StrTemp2 == NULL)
		{
			break;
		}
		if(StrTemp2 == StrTemp1)
		{
			continue;
		}

		strValue = (char *)malloc(sizeof(char) * (StrTemp2 - StrTemp1 + 1));
		strncpy(strValue, StrTemp1, StrTemp2 - StrTemp1);
		strValue[StrTemp2 - StrTemp1] = '\0';
		strValues[intCount] = strValue;

		StrTemp2 += strlen(StrSeparator);
		StrTemp = strCASEstr(StrTemp2, StrName);
		intCount ++;
	}
	strValues[intCount] = NULL;
	return strValues;
}

int freeMultiValue(char **strValues)
{
	char *strTemp;
	int i = 0;

	if(strValues == NULL)
	{
		return 0;
	}
	while(strValues[i])
	{
		strTemp = strValues[i];
		free(strTemp);
		i ++ ;
	}
	free(strValues);
	return 0;
}
//2013/7/15 11:21 
