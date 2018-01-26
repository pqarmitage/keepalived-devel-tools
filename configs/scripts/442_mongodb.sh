#!/bin/bash

RETFILE=${0##*/}
RETFILE=${RETFILE%.sh}
RETFILE=/tmp/${RETFILE%_script}.ret

echo "$(date +"%a %b %e %X %Y"): '$0' '$1' '$2' '$3' '$4'" >>/tmp/keepalived_script.log

ret=$(cat $RETFILE)

exit $ret
