#!/bin/bash
TIMEOUT=0.01

[[ -f /tmp/1364b.tim ]] && TIMEOUT=$(cat /tmp/1364b.tim)

exec 5<>/dev/tcp/localhost/"$1" && /usr/bin/timeout $TIMEOUT /usr/bin/echo -e "lua box.info.status\nexit" >&5 && /usr/bin/timeout 3 /usr/bin/grep -q "$2" <&5
