#!/bin/bash

if [[ $# -ge 1 && $1 != GROUP && $1 != INSTANCE ]]; then
	NAME=$1
else
	NAME=${0##*/}
	NAME=${NAME%.sh}
fi

echo "$(date +"%a %b %e %X %Y")": \[$PPID:$$\] $* >>/tmp/$NAME.log

exit 0
