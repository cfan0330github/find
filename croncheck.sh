#! /bin/sh


#for a in `cat /root/dom.list`
#do
# result=`dig @127.0.0.1 soa $a|grep SOA|grep myhostadmin|awk '{print $5}'`
# if [ -z "$result" ]; then
#     echo "bad domain $a"
# fi
#done



#cd "/root/BAIUDNS.update"
cat /0day/log.log|grep "add" -C 3|grep "domainname"|cut -d":" -f2 >/tmp/checkdom

while read line
do
  a=""
  a=`echo "$line"`
  a=${a%?}
  ./client add $a
 # echo $a
 #result=`dig @127.0.0.1 soa $a|grep SOA|grep myhostadmin|awk '{print $5}'`
#if [ -z "$result" ]; then
     #/root/BAIUDNS.update/client add $a
     #sleep 1
 #    echo "add domain $a"
 #fi
done< /tmp/checkdom
/bin/rm -f /tmp/checkdom