#!/bin/bash

IF_PREFIX=vif1
DIGITS=2
SCRIPT_TYPE=1
VRRP_PER_IF=1
TOTAL_IF=2

CUR_IFS=$(ip -n low link show | grep $IF_PREFIX | wc -l)
[[ $CUR_IFS -gt $TOTAL_IF ]] && CUR_IFS=$TOTAL_IF

k()
{
	SIG=${2:-TERM}

	if [[ -z $1 ]]; then
		kill -$SIG $(cat /run/keepalived.pid)
	else
		kill -$SIG $(cat /run/keepalived/$1/keepalived.pid)
	fi
}

write_config()
{
	NUM_IF=$1

    (
	if [[ $SCRIPT_TYPE -eq 1 ]]; then
		cat <<EOF
@low net_namespace low
@high net_namespace high

global_defs {
	vrrp_garp_master_delay 0
	vrrp_garp_master_repeat 1
	vrrp_garp_master_refresh 0
	vrrp_garp_master_refresh_repeat 2
}

EOF
	fi

	for i in $(seq $((TOTAL_IF - NUM_IF)) $((TOTAL_IF - 1))); do
		for j in $(seq 0 $((VRRP_PER_IF - 1))); do
			priority=100
			[[ $i = 0 && $j = 0 ]] && priority=99
			[[ $i = 0 && $j = 1 ]] && priority=101

			[[ $i -le 6 ]] && IP=$((i * VRRP_PER_IF + j + 3)) || IP=$((j  + 1))

			if [[ $SCRIPT_TYPE = 1 ]]; then
				SCRIPT="\"/home/quentin/keepalived/utils/configs/scripts/vs.sh VI_${IF_PREFIX}$(printf "%${DIGITS}.${DIGITS}d" $i)_$((i * VRRP_PER_IF + j + 1))\""
			else
				SCRIPT=/usr/local/nbase/sbin/svrrp_notify
			fi

			cat <<EOF

vrrp_instance VI_${IF_PREFIX}$(printf "%${DIGITS}.${DIGITS}d" $i)_$((i * VRRP_PER_IF + j + 1)) {
	state BACKUP
	interface ${IF_PREFIX}$(printf "%${DIGITS}.${DIGITS}d" $i)
	use_vmac
	virtual_router_id $((i * VRRP_PER_IF + j + 1))
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
    ) >/tmp/vif.conf
}

mk_if()
{
	IF=$1

	ip -n low link add link eth0 ${IF_PREFIX}$(printf "%${DIGITS}.${DIGITS}d" $IF) type macvlan mode private
	ip -n low addr add 1.1.1.2$(printf "%2.2d" $IF)/24 brd 1.1.1.0 dev ${IF_PREFIX}$(printf "%${DIGITS}.${DIGITS}d" $IF)
	ip -n low link set ${IF_PREFIX}$(printf "%${DIGITS}.${DIGITS}d" $IF) up

	echo Created ${IF_PREFIX}$(printf "%${DIGITS}.${DIGITS}d" $IF)
}

del_if()
{
	IF=$1
	ip -n low link del ${IF_PREFIX}$(printf "%${DIGITS}.${DIGITS}d" $IF)
	echo Deleted ${IF_PREFIX}$(printf "%${DIGITS}.${DIGITS}d" $IF)
}

make_ifs()
{
	NUM_IF=$1

	if [[ $NUM_IF -gt $CUR_IFS ]]; then
		for i in $(seq $((TOTAL_IF - NUM_IF)) $((TOTAL_IF - 1 - CUR_IFS)) ); do
			mk_if $i
		done
	elif [[ $NUM_IF -lt $CUR_IFS ]]; then
		for i in $(seq $((TOTAL_IF - CUR_IFS)) $((TOTAL_IF - 1 - NUM_IF)) ); do
			del_if $i
		done
	fi

	CUR_IFS=$NUM_IF
}

if [[ $1 = i ]]; then
	write_config $TOTAL_IF
	make_ifs $TOTAL_IF

	exit 0
fi

while [ 1 ]; do
	for i in $(seq $((TOTAL_IF -1)) -1 0); do
		echo Reducing to $i
		write_config $i
		make_ifs $i
		sleep 2

		k low HUP
		sleep 5

		k low USR1
		sleep 1
		grep -q "tate = FAULT" /tmp/keepalived.data
		[[ $? -eq 0 ]] && echo -e "\aGot fault state" && exit 0
	done

	for i in $(seq 1 $TOTAL_IF); do
		echo Increasing to $i
		write_config $i
		make_ifs $i
		sleep 2

		k low HUP
		sleep 5

		k low USR1
		sleep 1
		grep -q "tate = FAULT" /tmp/keepalived.data
		[[ $? -eq 0 ]] && echo -e "\aGot fault state" && exit 0
	done
done
