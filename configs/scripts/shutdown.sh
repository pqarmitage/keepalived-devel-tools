#!/bin/bash

echo "$(date +"%a %b %e %X %Y")": \[$PPID:$$\] >>/tmp/$(basename $0 .sh).log
