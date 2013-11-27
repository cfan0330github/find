#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(void){

  char *p;
  char s[]="test\n";
  p=malloc(20*sizeof(char));
  memset(p,0,20*sizeof(char));
  memcpy(p,s,strlen(s));
  printf("s length %d,p length is %d\n",strlen(s),strlen(p));
  exit(0);
}
