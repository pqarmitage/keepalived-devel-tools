#!/bin/bash

if [[ $# -ge 1 ]]; then
	NAME=$1
else
	NAME=${0##*/}
	NAME=${NAME%.sh}
fi

echo $(date): \[$$\] $* >>/tmp/$NAME.log

if [[ -f /tmp/$NAME.sig ]]; then
	SIG=$(cat /tmp/$NAME.sig)
	kill -$SIG $$

	sleep 5
fi

if [[ -f /tmp/$NAME.slp ]]; then
	SLEEP=$(cat /tmp/$NAME.slp)

	sleep $SLEEP
fi

[[ ! -f /tmp/$NAME.ret ]] && echo 0 >/tmp/$NAME.ret

RET=$(cat /tmp/$NAME.ret)

if [[ $RET -ge 3 && $RET -le 15 ]]; then
	 echo $((RET - 1)) >/tmp/$NAME.ret
	 sleep $RET
fi

exit $RET
