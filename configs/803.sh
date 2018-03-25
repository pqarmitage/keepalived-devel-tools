#!/bin/bash

if [[ -z $1 ]]; then
	NETNS_PREFIX=
else
	NETNS_PREFIX="ip netns exec $1"
fi

NUM=${2:-16}
$NETNS_PREFIX ip link add bond0 type bond mode 802.3ad miimon 100 lacp_rate fast
# $NETNS_PREFIX ip addr add 192.168.5.1/24 brd + dev bond0
$NETNS_PREFIX ip link set bond0 up
for IF in eth6 eth7; do
	$NETNS_PREFIX ip link set $IF down
	$NETNS_PREFIX ip link set $IF master bond0
	$NETNS_PREFIX ip link set $IF up
done

LAST_IF=$((10 + $NUM - 1))
for n in $(seq 10 $LAST_IF); do
	$NETNS_PREFIX ip link add link bond0 dev bond0.$n type vlan id $n
	$NETNS_PREFIX ip link set bond0.$n up
done

[[ $0 =~ / ]] && DIR=${0%/*}/ || DIR=
cp -p ${DIR}803.conf.tpl ${DIR}803.conf
(
for n in $(seq 10 $LAST_IF); do
	echo
       	echo \$NUM=$n
	echo \$VI
done
) >> ${DIR}803.conf
