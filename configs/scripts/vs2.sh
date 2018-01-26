#!/bin/bash

NAME=${0##*/}
NAME=${NAME%.sh}

echo $(date): \[$PPID:$$\] $* >>/tmp/$NAME.log

[[ ! -f /tmp/$NAME.ret ]] && echo 0 >/tmp/$NAME.ret

RET=$(cat /tmp/$NAME.ret)

if [[ $RET -ge 3 && $RET -le 15 ]]; then
	 echo $((RET - 1)) >/tmp/$NAME.ret
	 sleep $RET
fi

exit $RET
