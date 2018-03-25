#!/bin/bash
func_script() {
logger "Inside check script"
if [ $# -ne 3 ]
then
	echo "Insufficient arguments"
	exit 0
fi
for i in "${cla[@]}"
do
	echo $i
	\${_PWD}/./$i.sh
	retval=$?
	echo $retval
	logger "process:$i Retval:$retval"
	if [ $retval -eq 0 ]
	then
		logger "$i succeeded"
		count=$((count+1))
	fi
done
logger "Count :$count"
if [ $count -eq 3 ]
then
	retval=0
	logger "Check_myscript Retval:$retval"
	return $retval
else
	retval=1
	logger "Check_myscript Retval:$retval"
	return $retval
	echo "retval:$retval"
fi
}
cla=("$@")
func_script $@
