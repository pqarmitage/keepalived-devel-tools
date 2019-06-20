#!/bin/sh

PATH=/bin:/sbin:/usr/bin:/usr/sbin

umask 022

TYPE=$1
NAME=$2
STATE=$3

logger -t keepalived "Transition to state '$STATE' on VRRP instance '$NAME'."
