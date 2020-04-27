#!/bin/bash
exec 5<>/dev/tcp/localhost/"$1" && /usr/bin/timeout 3 /usr/bin/echo -e "lua box.info.status\nexit" >&5 && /usr/bin/timeout 3 /usr/bin/grep -q "$2" <&5
