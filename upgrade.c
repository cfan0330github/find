#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define CONFIG "/etc/named.conf"


void getDomain(char *buf,char *strDomain){
                char *p1,*p2;
                p1=buf;
                while(*p1==' ') p1++;
                p1=strchr(p1,' ');
                while(*p1==' ') p1++;
                p2=strrchr(p1,'.');

                if (p1 && p2 && (p2-p1>0)){
                                strncpy(strDomain,p1,p2-p1);
					}
};

void getFile(char *buf,char *filename){
		char *p1,*p2;
		p1=strchr(buf,'"');
		++p1;
		p2=strrchr(buf,'"');
		if (p1 && p2 && (p2-p1>0)){
			strncpy(filename,p1,p2-p1);
		}
}

void upzone(char *path){
        FILE *zfile;
        FILE *tfile;
        char tmpfile[512],strDomain[512],buf[1024];
        char *header="$TTL 900\n$ORIGIN %s.\n@\tIN\tSOA\tns1.myhostadmin.net. dnsconct.myhostadmin.net. (\n\t\t\t\t2009042201 \n\t\t\t\t3600\n\t\t\t\t300\n\t\t\t\t1209600\n\t\t\t\t3600\n\t\t\t\t)\n\t\t\t3600\tNS\tns1.myhostadmin.net.\n\t\t\t3600\tNS\tns2.myhostadmin.net.\n\t\t\t3600\tNS\tns3.myhostadmin.net.\n\t\t\t3600\tNS\tns4.myhostadmin.net.\n\t\t\t3600\tNS\tns5.myhostadmin.net.\n\t\t\t3600\tNS\tns6.myhostadmin.net.\n";
        int blWrite=0;

        sprintf(tmpfile,"%s.tmp",path);
        zfile=fopen(path,"r");
        tfile=fopen(tmpfile,"w");

        if (zfile==NULL||tfile==NULL){
                        printf("upgrade zone %s failed...\n",path);
                        return;
        }

        while(fgets(buf,sizeof(buf),zfile)){
                if (!blWrite && strstr(buf,"$ORIGIN")){
								bzero(strDomain,sizeof(strDomain));
                                getDomain(buf,strDomain);
                                continue;
                }

                if (!blWrite && strstr(buf,"ns2.myhostadmin.net.") && strstr(buf,"3600") && strstr(buf,"NS")){
                        blWrite=1;
                        fprintf(tfile,header,strDomain);
                        continue;
                }

                if (blWrite)
                        fputs(buf,tfile);
        }

        fclose(tfile);
        fclose(zfile);
        rename(tmpfile,path);
}


main(){
	FILE *zonecfg;
	char *p1,*p2,buf[1024],filename[1024];
	zonecfg=fopen(CONFIG,"r");
	if (zonecfg==NULL){
			puts("read zone config error.");
			return;
	}
	while(fgets(buf,sizeof buf,zonecfg)){
		if (strstr(buf,"file")){
			bzero(filename,sizeof(filename));
			getFile(buf,filename);
			if (access(filename,F_OK)==0){
				printf("updateing %s\n",filename);
				upzone(filename);
			}

		}
	}


}
