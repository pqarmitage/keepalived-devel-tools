#!/bin/bash

set -x
(
echo $(whoami)
if [ $(systemctl is-active nfs-server.service) == "active" ]
then
  exit 0
else
  exit 1
fi

) >> /tmp/checknfs.log 2>&1
