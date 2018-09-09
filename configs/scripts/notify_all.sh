#!/bin/bash

if [[ $# -ge 1 && $1 != GROUP && $1 != INSTANCE ]]; then
	NAME=$1
else
	NAME=${0##*/}
	NAME=${NAME%.sh}
fi

echo -n "$(date +"%a %b %e %X %Y")": \[$PPID:$$\] >>/tmp/$NAME.log
for i in $(seq 1 $#); do
	echo -n " '$1'" >>/tmp/$NAME.log
	shift
done
echo >>/tmp/$NAME.log

exit 0
