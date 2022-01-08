#!/bin/bash

PID_FILE=/run/keepalived/keepalived.pid

echo $(date) run $(cat $PID_FILE) $* >>/tmp/reload.log

to_master() {
     kill -HUP $(cat $PID_FILE)
}

to_backup() {
     kill -HUP $(cat $PID_FILE)
}

to_fault() {
     kill -HUP $(cat $PID_FILE)
}

to_check() {
     kill -HUP $(cat $PID_FILE)
}

exit 0
case $1 in
     check)
          to_check
           ;;
     master)
          to_master
           ;;
     backup)
          to_backup
           ;;
     fault)
          to_fault
           ;;
      *)
           echo
           ;;
esac
