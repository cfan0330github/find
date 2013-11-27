
all:	linuxdns

tools_ssllib.o:	tools_ssllib.c
	gcc -g -c  -m64 tools_ssllib.c 
baiudnsmodule.o:	baiudnsmodule.c
	gcc -g  -c -m64  baiudnsmodule.c 
cron.o:    cron.c
	gcc -c -m64 cron.c
cmdlog.o:	cmdlog.c
	gcc -g  -c -m64   cmdlog.c
linuxdns:	 tools_ssllib.o baiudnsmodule.o server.c md5.c md5.o cmdlog.o cron.o
	gcc -g -s -m64 -o $@ server.c md5.c tools_ssllib.o baiudnsmodule.o  cmdlog.o cron.o
client:       client
	gcc -c -m64   client.c md5.c tools_ssllib.c

clean:
	rm  linuxdns tools_ssllib.o baiudnsmodule.o cmdlog.o md5.o cron.o
