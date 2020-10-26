#!/bin/bash
#systemctl restart snmpd
killall -TERM snmpd
echo $(date) killall returned $? >>/tmp/kill_snmpd.log
sleep 0.1
ip netns exec low snmpd -LS0-6d
echo $(date) started snmpd >>/tmp/kill_snmpd.log
exit 0
