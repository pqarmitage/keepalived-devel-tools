#!/bin/bash

TMPDIR=/tmp

# TMPDIR=/etc/keepalived/tmp

if [[ $# -ge 1 ]]; then
	NAME=$1
else
	NAME=${0##*/}
	NAME=${NAME%.sh}
fi

echo -n "$(date +"%a %b %e %X %Y")": \[$PPID:$$\] >>$TMPDIR/$NAME.log
for i in $(seq 1 $#); do
	echo -n " '$1'" >>$TMPDIR/$NAME.log
	shift
done
echo >>$TMPDIR/$NAME.log
id >>/$TMPDIR/$NAME.log

if [[ -x $TMPDIR/$NAME.scr ]]; then
	echo "  Executing $TMPDIR/$NAME.scr" >>$TMPDIR/$NAME.log

	$TMPDIR/$NAME.scr
fi

if [[ -f $TMPDIR/$NAME.sig ]]; then
	SIG=$(cat $TMPDIR/$NAME.sig)
	kill -$SIG $$
	echo "  Sent signal $SIG" >>$TMPDIR/$NAME.log

	sleep 5
fi

if [[ -f $TMPDIR/$NAME.slp ]]; then
	SLEEP=$(cat $TMPDIR/$NAME.slp)
	echo "  Sleeping $SLEEP seconds" >>$TMPDIR/$NAME.log

	sleep $SLEEP
fi

[[ ! -f $TMPDIR/$NAME.ret ]] && echo 0 >$TMPDIR/$NAME.ret

RET=$(cat $TMPDIR/$NAME.ret)

if [[ $RET -ge 3 && $RET -le 15 ]]; then
	echo $((RET - 1)) >$TMPDIR/$NAME.ret
	echo "  Ret sleeping $RET seconds" >>$TMPDIR/$NAME.log

	sleep $RET
fi

echo "  Returning $RET" >>$TMPDIR/$NAME.log

exit $RET
