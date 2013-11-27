#!/bin/bash

SIGSTR="westwinok"
G_SERIAL_file="/tmp/tmp$$.tmp"
md5(){
        md5str=$1
        export MD5STR=$md5str
        RET=$(/usr/bin/php /var/named/a.php)
        md5lines=(${RET[@]})

count=0
md5ret=""
while [ $count -lt ${#md5lines} ]
do
        if [ ${#md5lines[$count]} -eq 32 ];then
                md5ret=${md5lines[$count]}
                break
        fi
        let count=count+1
done

}

GetSerial(){
        file=$1
        if [ ! -f "$file" ];then
                G_SERIAL=$(date +%Y%m%d%H)
                echo -n $G_SERIAL>$G_SERIAL_file
                return
        fi

        OIFS=$IFS
        IFS=
        FLAG=0
        cat $file|while read line
        do
                if [ $FLAG -eq 1 ];then
                        pos=$(expr index "$line" ";")
                        if [ $pos -gt 0 ];then
                                pos=$(( $pos-1 ))
                                line=${line:0:$pos}
                        fi
                        line=$(echo $line|tr -d [:blank:])

                        G_SERIAL=$(expr $line + 10)

                        IFS=$OIFS
                        echo -n $G_SERIAL>$G_SERIAL_file
                        return
                else
                  if [[ "$line" =~ "dnsconct" ]];then
                        FLAG=1
                  fi
                fi
        done
        IFS=$OIFS
}

UpdateZone()
{
 str=$1
 matchStr="/${str}.name"
 lines=$(fgrep "$matchStr" /etc/named.conf)

 if [ -z "$lines" ];then
        echo "505 $str not exists."
        exit
 fi

 pos1=$(expr index "$lines" "\"")

 if [ $pos1 -eq 0 ];then
        echo "506 $str config error"
        exit
 fi

 lines=${lines:$pos1}
 pos1=$(expr index "$lines" "\"")

 if [ $pos1 -eq 0 ];then
        echo "507 $str config error2"
        exit
 fi

 pos1=$((pos1-1))
 lines=${lines:0:$pos1}



 if [ -f "$lines" ];then
		/usr/sbin/rndc freeze $str
	 	GetSerial "$lines"

		if [ -f "$G_SERIAL_file" ];then
			sno=$(cat $G_SERIAL_file)
			rm -f "$G_SERIAL_file"
		else
			sno=$(date +%Y%m%d%H)
		fi

		if [ -f "${lines}.jnl" ];then
			rm -f "${lines}.jnl"
		fi
		echo -n >"$lines"
		IFS=
		read nextLine
		while [ "$nextLine" != "__END__" ]
		do
		  if [[ $nextLine =~ "__SN__" ]];then
			echo  "					${sno}; Serial" >>"$lines"
		  else
			echo  $nextLine >>"$lines"
		  fi
			read nextLine
		done

		/usr/sbin/rndc thaw $str >/dev/null
		echo "200 ok $lines update success"
 else
	        echo "508 $lines not exists."
 fi
}


ZoneCheck()
{
 str=$1
 matchStr="/${str}.name"
 lines=$(fgrep "$matchStr" /etc/named.conf)

 if [ -z "$lines" ];then
        echo "505 $str not exists."
        exit
 fi

 pos1=$(expr index "$lines" "\"")

 if [ $pos1 -eq 0 ];then
        echo "506 $str config error"
        exit
 fi

 lines=${lines:$pos1}
 pos1=$(expr index "$lines" "\"")

 if [ $pos1 -eq 0 ];then
        echo "507 $str config error2"
        exit
 fi

 pos1=$((pos1-1))
 lines=${lines:0:$pos1}

 if [ -f "$lines" ];then
	/usr/sbin/named-checkzone $str $lines 2>&1	
 else
        echo "508 $lines not exists."
 fi
}

GetZone()
{
 str=$1
 matchStr="/${str}.name"
 lines=$(fgrep "$matchStr" /etc/named.conf)

 if [ -z "$lines" ];then
        echo "505 $str not exists."
        exit
 fi

 pos1=$(expr index "$lines" "\"")

 if [ $pos1 -eq 0 ];then
        echo "506 $str config error"
        exit
 fi

 lines=${lines:$pos1}
 pos1=$(expr index "$lines" "\"")

 if [ $pos1 -eq 0 ];then
        echo "507 $str config error2"
        exit
 fi

 pos1=$((pos1-1))
 lines=${lines:0:$pos1}

 if [ -f "$lines" ];then
	rndc freeze $str
        cat $lines
	rndc thaw $str
 else
        echo "508 $lines not exists."
 fi
}

logfile="/var/named/hello.log"
cmd=""
echo -n "LOG:$(date)">>$logfile
#counti=1
#authed=false
#while true
# do
		read ucmd 
		cmdLen=${#ucmd}
		if [ ${cmdLen} -gt 100 ];then
			echo "777 line too long!"
			exit
		fi
#		echo $ucmd >>"/var/named/cmd.txt"
		let "cmdLen= cmdLen - 1"
	 	ucmd=${ucmd:0:$cmdLen}
		echo ": USER TYPE:$ucmd" >>$logfile

		strPos=$(expr index "$ucmd" ":")
		if [ $strPos -eq 0 ]
		 then
			echo "500 Invalid Format"
			exit
		fi
		strPos=$(( strPos-1 ))
		cmdStr=${ucmd:0:$strPos}
		strPos=$((strPos+1))
		domain=${ucmd:$strPos}
		
		if [ -z "$domain" ];then
			echo "502 domain is null"
			exit
		fi
		
		case "$cmdStr" in
			(get|GET)
				GetZone $domain
				exit;;
			(check|CHECK)
				ZoneCheck $domain
				exit;;
			(update|UPDATE)
				pos1=$(expr index $domain ":")
					if [ $pos1 -eq 0 ];then
						echo "504 md5 missing"
						exit
					fi
					tempstr=$domain
					let pos1=pos1-1
					domain=${domain:0:$pos1}
					let pos1=pos1+1
					md5code=${tempstr:$pos1}
					
					md5 "domain${domain}${SIGSTR}"
					if [ "$md5code" != "$md5ret" ];then
						echo "506 auth failure"
						exit
					fi
					UpdateZone $domain
					exit;;
			(*)
				echo "504 Invalid Command"
				exit;;
		esac
#done

