#!/bin/bash
find_pid() {
	pname=$1
	echo $pname
	pid=$(pgrep -x "$pname")
	echo $pid
	if ! kill -0 $pid
	then
		retval=1
	else
		retval=0
	fi
	logger "Inside Notify_fault: Retval:$retval "
	echo $retval
	if [ $retval -eq 1 ]
	then
		logger "Creating Process pname:$pname"
		/home/netstorm/work/lolika/./$pname &
		pid=$(pgrep -x "$pname")
		logger "PID=$pid"
	fi
}
find_pid $1
