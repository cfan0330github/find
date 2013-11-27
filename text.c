#include<stdio.h>
#include"server.h"

void do_func(int signo){
  printf("type word %d\r\n.",a);

 // goto domain;
  return;
}

int main(void){

NEWDOM* p;
cmdlog *plog;
FILE * f_lst;
char fbuf[512]={0};
msgbuf tmsg;
int result;
int TIMEOUT=5;
time_t curt,tmpt,div;
int a=3;

/*while(1) {
//domain:
signal(SIGALRM,do_func);
alarm(TIMEOUT);
printf("before msgrcv.\r\n");
result=msgrcv(0,&tmsg,sizeof(msgbuf)-sizeof(long),1,0);
printf("after msgrcv.\r\n");
}
*/

tmpt=time(0L);
sleep(3);
curt=time(0L);
div=curt-tmpt;
printf("div is %d,long is %d,time_t is %d.\n",div,sizeof(long),sizeof(time_t));
return 0;


}
//2013/6/11 17:18 