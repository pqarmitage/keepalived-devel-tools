#!/bin/bash

if [[ $(cut -f1 -d '.' /proc/uptime) -gt 300 ]]; then
  exit 0
fi

exit 1
