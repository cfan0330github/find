#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>


int m_tag;

void dogetchild(int sigb){
   printf("baby die,reCreate baby.\n");
   m_tag=0;
   return;
}

void dolog(int log){
   printf("pid is %d, parent id is %d,program crashed.\n",getpid(),getppid());
   //为自己的进程产生core 文件，又不想退出这个进程
   //if (0==fork())   abort();//||(*((void*)0) = 42);
   kill(getppid(),SIGALRM);
   //abort();
   exit(0);
   return;
}

pid_t createbaby(void){
    pid_t newb; 
	signal(SIGSEGV,dolog);
	if(0==(newb=fork())){
	    int i=3;
		int* p=NULL;
		int tag=0;
	   while(i){
		//signal(SIGSEGV,dolog);
	     sleep(1);
	     printf("baby is here, i is %d.\n",i);
	     i--;
		 tag++;
	    }
	   if (3==tag) tag=0,*p=10;
	   sleep(5);
	 }else
		 return newb;
}


int main()
{
pid_t forker;

	int status=0;
	m_tag=0;
	while(1){
	puts("gogogo..\n");
		
	if (!m_tag)
	{
		forker=createbaby();		
	    m_tag=1;
	}
	waitpid(0,&status,WNOHANG);
	signal(SIGALRM,dogetchild);
    sleep(2);
	}
}