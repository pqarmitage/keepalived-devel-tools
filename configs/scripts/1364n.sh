#!/bin/bash
/usr/bin/echo "lua box.info.status" | /usr/bin/nc -w 3 $3 "$1" | /usr/bin/grep -q "$2"
