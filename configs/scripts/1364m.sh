#!/bin/bash
/usr/bin/timeout 3 bash -x <<EOF
exec 5<>/dev/tcp/$3/"$1" && /usr/bin/timeout 3 /usr/bin/echo -e "lua box.info.status\nexit" >&5 && /usr/bin/timeout 3 /usr/bin/grep -q "$2" <&5
EOF
