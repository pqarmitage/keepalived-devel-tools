#!/bin/bash

echo $(date) run >>/tmp/run.log
status=$(ps aux|grep haproxy | grep -v grep | grep -v bash | wc -l)
if [ "${status}" = "0" ]; then
    haproxy -f /usr/local/haproxy/haproxy.cfg

    status2=$(ps aux|grep haproxy | grep -v grep | grep -v bash |wc -l)
    if [ "${status2}" = "0"  ]; then
        systemctl stop keepalived.service
        exit 1
    else
    exit 0
    fi
fi
