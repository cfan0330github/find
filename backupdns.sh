#!/bin/bash

strDate=`date +%H%M`

cd /bkup

cp -f /etc/named.conf /bkup/conf/named.conf${strDate}
