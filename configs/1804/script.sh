#!/bin/bash

NAME=script
PID_FILE=/run/keepalived/keepalived.pid

to_master() {
     echo kill -HUP $(cat $PID_FILE)    >>/tmp/$NAME.log
     kill -HUP $(cat $PID_FILE)   
     exit 0    
}

to_backup() {
     echo kill -HUP $(cat $PID_FILE)    >>/tmp/$NAME.log
     kill -HUP $(cat $PID_FILE)      
     exit 0 
}

to_fault() {
     echo kill -HUP $(cat $PID_FILE)    >>/tmp/$NAME.log
     kill -HUP $(cat $PID_FILE)  
     exit 0    
}

to_check() {
     exit 0 
}


if [[ $1 != to_check ]]; then
	echo -n "$(date +"%a %b %e %X %Y")": \[$PPID:$$\] >>/tmp/$NAME.log
	echo $* >>/tmp/$NAME.log
fi

case $1 in 
     to_check)
          to_check
           ;;
     to_master)
          to_master
           ;;
     to_backup)
          to_backup
           ;;
     to_fault)
          to_fault
           ;;
      *)
           echo
           ;;
esac
