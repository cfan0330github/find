#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
//添加队列
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>



#define MAXMSGSIZE  1024
#define	SERVERMODE	1
#define CLIENTMODE	2

#define	CHG_SUCCESS		0
#define	CHG_EUPDTPASSWD	-3
#define	CHG_EBACKPASSWD	-4
#define	CHG_EOPENTMP	-5

#define	CHG_ELOCKP		-6
#define	CHG_ELOCKSHADOW	-7
#define	CHG_EULOCKP		-8


#define	MysqlDefaultUser "root"
#define MysqlDefaultPwd  "syourpasswords"
#define MysqlDefaultDb  "baiuvhost"


#define DEFAULTROOT   "/named"
#define DEFAULTCONFIG  "/etc/named.conf"
#define UPDATECONFIG   "/etc/lwresd.conf"
#define DEFAULTMPCONFIG  "/etc/named.conf.swf"
#define TMPUPDATECONFIG   "/etc/lwresd.conf.swf"
#define DEFAULTREPLACEVALUE  "//add new zone file here"
#define DEFAULTREPLACEOPT    "//end options here"
#define DEFAULTTTL  "3600"

#define DEFAULTDNSSERVER "ns1.myhostadmin.net"

#define DEFAULTMXTYPE  "MX"
#define DEFAULTCNAMETYPE  "CNAME"
#define DEFAULTATYPE  "A"
#define DEFAULTTXTTYPE  "TXT"


#define	MAXBUFFERLEN	102401
#define USERID		"jiaGji"
#define PASSWORD	"abac3C4edc"

#define HTTPVHOME	"/home/bchttpd/"

#define QUOTADEVICE	"/dev/hdc2"
#define DefaultBaseHome "/home"
#define	DEFAULTIP	"220.166.64.222"

#define	DefaultGroup "clients"
#define DefaultApacheUser "apache"

#define	valid_com_num	3

#define	REGMSG_200	"200 Command completed successfully\r\n.\r\n"
#define REGMSG_200A     "200 Command completed successfully!rndc reload ok\r\n.\r\n"
#define	REGMSG_200B	"200 Command completed successfully\r\nProtocol: BURRP 0.1\r\n.\r\n"
#define REGMSG_200C	"200 Command completed successfully\r\nspace:%d\r\n.\r\n"
#define	REGMSG_504	"504 Missing required attribute\r\nMessage: Missing Attribute: %s\r\n.\r\n"
#define	REGMSG_500	"500 Invalid command name\r\n.\r\n"
#define	REGMSG_505	"505 Invalid attribute value syntax\r\n.\r\n"
#define	REGMSG_505B	"505 Invalid attribute value syntax\r\nMessage: Invalid attribute: %s\r\n.\r\n"
#define	REGMSG_520	"520 Add user account failed\r\n.\r\n"
#define	REGMSG_521	"521 Set disk quota failed\r\n.\r\n"
#define	REGMSG_421	"421 Some of data sent are incorrect, system can not operate successfully\r\n.\r\n"
#define	REGMSG_529	"529 Operating Entity is not exist or Authorization failed\r\n.\r\n"
#define	REGMSG_507	"507 Invalid command format\r\n.\r\n"
#define REGMSG_531	"531 Authorization failed\r\n.\r\n"
#define	REGMSG_554	"554 User exist\r\n.\r\n"
#define REGMSG_530	"530 Database Connect Error\r\n.\r\n"
#define REGMSG_535	"535 Database query Error\r\n.\r\n"
#define REGMSG_555	"555 Domain already parked\r\n.\r\n"
#define	REGMSG_540	"540 Comand completed failed\r\n.\r\n"
#define REGMSG_545	"545 SiteName already stopped\r\n.\r\n"
#define REGMSG_601	"601 add Domain to named config err \r\n.\r\n"
#define REGMSG_602	"602 Create Domain config err \r\n.\r\n"
#define REGMSG_700	"700 Program is Under develop \r\n.\r\n"

#define REGMSG_603	"603 del Domain from named config err \r\n.\r\n"
#define REGMSG_604	"604 del Domain config err \r\n.\r\n"
#define REGMSG_211	"211 Domain exists config err \r\n.\r\n"
#define REGMSG_210	"210 Domain exists config err \r\n.\r\n"

#define DNS_1		"ns1.myhostadmin.net"
#define DNS_2		"ns2.myhostadmin.net"
#define DNS_3		"ns3.myhostadmin.net"
#define DNS_4		"ns4.myhostadmin.net"
#define DNS_5		"ns5.myhostadmin.net"
#define DNS_6		"ns6.myhostadmin.net"
#define ERRLOG      "/cmdsnd.log"

//传递消息
struct _MSGBUF
{
	long mtype;
	int ttl;
	int act;                                                                          //0:del, 1:add, 2:show
	char mtext[MAXMSGSIZE];
};

//需要添加的域名列表
struct _NEWDOM
{  
	 char  mtext[MAXMSGSIZE];
     struct _NEWDOM  *next; 
	 int tag;                        //0 就是无  1就是已存在,不用操作  (头指针的tag是当前需要更新的资源个数)
};

//服务器列表
struct _SERVERLIST
{
    char serip[16];
	struct _SERVERLIST   *nextip;
	int tag;                                                                          //(root->tag 表示是否已经运行定时: 0没有 1有)是否出现网络阻塞: 0 没有, 1 出现 2当前检查正常,准备重发命令
};

//发送日志出错记录
struct _CMDLOG
{
	char serverip[16];                                                                 //服务器IP地址
	time_t creatime;                                                                   //初次发送失败时间
	time_t modtime;                                                                    //最近一次发送失败时间
	char *domain;                                                                      //出错域名名称
	char act[3];                                                                       //操作方式:add:1,del:0,mod
	int status;                                                                        //仅头指针:(0:可操作,1不能操作),元素(2临时添加,3临时删除)
	char md5key[33];                                                                   //记录唯一ID
	struct _CMDLOG *nextlog;                                                           //下条记录位置
};


typedef struct _MSGBUF           msgbuf;
typedef struct _NEWDOM           NEWDOM;
typedef struct _SERVERLIST       serverlist;
typedef struct _CMDLOG           cmdlog;
typedef int bool;

struct _SERVERMODE                                                                     //服务器需要的所有链表
{
	serverlist  **sl;
	cmdlog      **cg;    
};

typedef struct _SERVERMODE  servermode;
servermode serverglobal;

#define TRUE 1;
#define FALSE 0;

//char *addNewDomain(char *strDomain,int intTTL);
//int do_md5(char * md5sources,char * md5recieve);
void writeErrlog(FILE *f_errLog, char *strFormat, ...);
void cron_check(int signo);
void initProcessor(char *serverlst);
int getDomaindir(char *strDomainPath,char *strDomain);
int getDomainSerial(char *strDomainSerial);
int addConfigFiles(char *strNewvalue, char *strDomain);
char *delDomain(NEWDOM *dom);
void getUGID(uid_t *uidNamed, gid_t *gidNamed, char *strUserName);
int unlockp(char *LockFile);
int delConfigFiles(char *strDomain);
void writeLog(FILE *fLog, char *strFormat, ...);
char *addDomain(NEWDOM *dom,int tag);
int sndMsg(char *sndmsg,char *serverip,int serverport);
void sndCmd(cmdlog *cmdlog_lst,serverlist *seriplst,msgbuf *gbuf);
bool mk_md5(char * md5sources,char * md5recieve);
static void cron_snd(int signo);
//static int checkzonef(char *zone,char *zonepathfile,FILE *f_conf);
NEWDOM* linked(NEWDOM* root,msgbuf *addmsg);
int checkzone(NEWDOM* zone);
int qid;

//2013/7/15 15:02 