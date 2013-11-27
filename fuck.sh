#! /bin/sh
for b in `cat /root/BAIUDNS.MOD/dir.txt` 
do
    mkdir /named/$b
    cd /named/$b
#    pwd
    for a in `cat /root/BAIUDNS.MOD/dir.txt`
          do 
                 mkdir $a
                 cd /named/$b/$a
 #                pwd
                 for c in `cat /root/dir.txt`
                 do
                      mkdir $c
                 done
                 cd .. 
         done 
         cd ..
done
