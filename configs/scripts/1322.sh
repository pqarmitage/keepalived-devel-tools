#!/bin/bash

SERVER=localhost
PORT=400

checkingport=`echo "quit" | telnet $SERVER $PORT | grep "Escape character is"`

result="$?"

if [ $result -eq 0 ]; then
echo "awesome!"
exit 0
else
echo "awwwww.."
exit 1
fi
