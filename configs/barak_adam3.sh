#!/bin/bash

if [[ .$1 = .pqa ]]; then
	IF_PREFIX=vif1
	DIGITS=2
	SCRIPT_TYPE=1

	cat <<EOF
@low net_namespace low
@high net_namespace high
EOF
else
	IF_PREFIX=vif1
	DIGITS=2
	SCRIPT_TYPE=0
fi

for i in $(seq 0 9); do
	for j in $(seq 0 9); do
		priority=100
		[[ $i = 0 && $j = 0 ]] && priority=99
		[[ $i = 0 && $j = 1 ]] && priority=101

		[[ $i -le 6 ]] && IP=$((i * 10 + j + 3)) || IP=$((j  + 1))

		if [[ $SCRIPT_TYPE = 1 ]]; then
			SCRIPT="\"\${_PWD}/scripts/vs.sh VI_${IF_PREFIX}$(printf "%${DIGITS}.${DIGITS}d" $i)_$((i * 10 + j + 1))\""
		else
			SCRIPT=/usr/local/nbase/sbin/svrrp_notify
		fi

		cat <<EOF

vrrp_instance VI_${IF_PREFIX}$(printf "%${DIGITS}.${DIGITS}d" $i)_$((i * 10 + j + 1)) {
	state BACKUP
	interface ${IF_PREFIX}$(printf "%${DIGITS}.${DIGITS}d" $i)
	use_vmac
	virtual_router_id $((i * 10 + j + 1))
	virtual_ipaddress {
		1.1.$((i + 1)).$IP/32 brd 1.1.$((i + 1)).0
	}
	priority $priority
	advert_int 1
	notify $SCRIPT
}
EOF

	done
done
