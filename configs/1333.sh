#!/bin/bash

NS=${1:-low}

for i in $(seq 101 356); do
	ip -n $NS link add link eth0 br$i type vlan id $i
	[[ $? -ne 0 ]] && echo Failed on $i && exit 1

	IP1=$((i / 128))
	IP2=$((i % 128))

	ip -n $NS addr add 10.${IP1}.${IP2}.1/24 brd + dev br$i
	[[ $? -ne 0 ]] && echo Failed on $i && exit 1

	ip -n $NS link set br$i up
	[[ $? -ne 0 ]] && echo Failed on $i && exit 1
done
