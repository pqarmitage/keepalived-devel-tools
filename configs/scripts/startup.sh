#!!/bin/bash

echo -n "$(date +"%a %b %e %X %Y")": \[$PPID:$$\] >>/tmp/$(basename $0 .sh).log
