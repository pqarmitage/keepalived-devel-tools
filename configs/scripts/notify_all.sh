#!/bin/bash

if [[ $# -ge 1 && $1 != GROUP && $1 != INSTANCE ]]; then
	NAME=$1
else
	NAME=${0##*/}
	NAME=${NAME%.sh}
fi

PARAMS=
for i in $(seq 1 $#); do
	PARAMS="$PARAMS $1"
	shift
done

echo "$(date +"%a %b %e %X %Y")": \[$PPID:$$\] $PARAMS >>/tmp/$NAME.log

exit 0
