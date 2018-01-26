#!/bin/bash

FILE=${0##*/}
FILE=/tmp/${FILE%%.sh}.ret

[[ ! -f $FILE ]] && echo 0 >$FILE
RET=$(cat $FILE)

exit $RET
