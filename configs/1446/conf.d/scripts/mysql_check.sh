#!/bin/bash
######################
# Small Service Wrapper
######################
/usr/lib/nagios/plugins/check_tcp -H $1 -p 3306 > /dev/null 2>&1 && exit 1
/usr/lib/nagios/plugins/check_tcp -H $2 -p 3306 > /dev/null 2>&1 && exit 0

exit 1
