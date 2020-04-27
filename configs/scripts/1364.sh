#!/bin/bash
/usr/bin/echo "lua box.info.status" | /usr/bin/nc -w 3 localhost "$1" | /usr/bin/grep -q "$2"
