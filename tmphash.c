#include <stdio.h>
#include  "md5.h"

char hashstr[13]="this is test.";

md5_state_t state;
md5_byte_t digest[16];


main(void)
{
  int i;
  md5_init(&state);
  md5_append(&state,(const md5_byte_t *)hashstr,strlen(hashstr));
  md5_finish(&state,digest);
  for (i=0;i<16;i++)
   {
       printf("hash values:%c\r\n",digest[i]);
   } 
}
