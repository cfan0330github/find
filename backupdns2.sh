#!/bin/bash

strDate=`date +%H`

cd /bkup

/bin/tar czPf named.all${strDate}.tar.gz /named/* /etc/named.conf /var/named

/usr/bin/ftp -n <<EOF
open dnsback.gotoip4.com
user dnsback wskihia
pass off
prompt
bin
put named.all${strDate}.tar.gz
bye
EOF

