#!/bin/bash

if [[ -n $1 ]]; then
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

[[ ! -f /tmp/$NAME.ret ]] && echo 0 >/tmp/$NAME.ret

RET=$(cat /tmp/$NAME.ret)

if [[ $RET -ge 3 && $RET -le 15 ]]; then
	 echo $((RET - 1)) >/tmp/$NAME.ret
	 sleep $RET
fi

exit $RET
