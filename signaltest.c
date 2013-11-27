#include <sys/types.h>
#include <pwd.h>
#include <stdio.h>
#include<time.h>
#include<unistd.h>
#include<signal.h>

void handler(){
 printf("timeup!");
 return;
 }


main(void)
{
	signal(SIGALRM,handler); 
    //signal(SIGALRM,handler);
       alarm(1);
	while (1)
	{
	  printf("fuyun\n");
      sleep(1);
	}
 return 0;
}

/*
void handler() {
printf("hellon");
}*/
/*
main()
{
int i;
signal(SIGALRM,handler);
alarm(5);
for(i=1;i<7;i++){
printf("sleep %d ...\n",i);
sleep(1);
}
}*/