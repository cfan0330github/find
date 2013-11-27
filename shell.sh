#! /bin/sh

basedir=/root
updatefile=gonow.tar.gz



  
   /bin/rm -rf $basedir/BAIUDNS.old
   mv $basedir/BAIUDNS.update $basedir/BAIUDNS.old

   mv $basedir/BAIUDNS.crond $basedir/BAIUDNS.update
   cd $basedir/BAIUDNS.update
   mv ./linuxdns /etc/init.d/linuxdns  
   ip=`ifconfig|grep eth0 -C 2|grep "inet addr"|cut -d":" -f2|awk '{print $1}'`
   if [ -n "$ip" ]; then 
     echo "$ip">$basedir/BAIUDNS.update/conf/access.txt
   fi
   echo "61.139.126.202" >>$basedir/BAIUDNS.update/conf/access.txt
   /bin/cp -f $basedir/BAIUDNS.old/conf/log.log $basedir/BAIUDNS.update/conf/log.log 
   

   cd $basedir/BAIUDNS.update
   make clean
   make
   service linuxdns restart
   /bin/rm -f $basedir/$updatefile
