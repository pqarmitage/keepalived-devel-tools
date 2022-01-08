#!/bin/bash

NETNS_CMD="ip netns exec low"

# At start, interface ha exists and is up, interface vlan8 exists but is down and doesn't have an IP address, vlan1 and wan do not exist

if [[ $1 = start ]]; then
	$NETNS_CMD ip link add ha link eth3 type vlan id 3
	$NETNS_CMD ip add add 192.168.20.2/24 dev ha
	$NETNS_CMD ip link set ha up

	$NETNS_CMD ip link add vlan8 link eth4 type vlan id 4

	exit
fi

$NETNS_CMD ip link add wan link eth2 type vlan id 2

$NETNS_CMD ip link add vlan1 link eth1 type vlan id 1
$NETNS_CMD ip addr add 10.10.1.252/24 brd + dev vlan1
$NETNS_CMD ip addr add 10.10.8.252/24 brd + dev vlan8
$NETNS_CMD ip addr add 20.20.20.252/24 brd + dev wan

$NETNS_CMD ip link set wan up
$NETNS_CMD ip link set vlan1 up
$NETNS_CMD ip link set vlan8 up
