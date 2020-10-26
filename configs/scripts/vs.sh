#!/bin/bash

if [[ $# -ge 1 ]]; then
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

if [[ -x /tmp/$NAME.scr ]]; then
	echo "  Executing /tmp/$NAME.scr" >>/tmp/$NAME.log

	/tmp/$NAME.scr
fi

if [[ -f /tmp/$NAME.sig ]]; then
	SIG=$(cat /tmp/$NAME.sig)
	kill -$SIG $$
	echo "  Sent signal $SIG" >>/tmp/$NAME.log

	sleep 5
fi

if [[ -f /tmp/$NAME.slp ]]; then
	SLEEP=$(cat /tmp/$NAME.slp)
	echo "  Sleeping $SLEEP seconds" >>/tmp/$NAME.log

	sleep $SLEEP
fi

[[ ! -f /tmp/$NAME.ret ]] && echo 0 >/tmp/$NAME.ret

RET=$(cat /tmp/$NAME.ret)

if [[ $RET -ge 3 && $RET -le 15 ]]; then
	echo $((RET - 1)) >/tmp/$NAME.ret
	echo "  Ret sleeping $RET seconds" >>/tmp/$NAME.log

	sleep $RET
fi

echo "  Returning $RET" >>/tmp/$NAME.log

exit $RET
