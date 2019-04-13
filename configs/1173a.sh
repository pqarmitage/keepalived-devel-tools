#!/bin/sh

# Run as: sh ak.sh -d /etc/keepalived/conf -v 22.22.1.1_80_300 -r 22.22.221.1_80_100 -h tcp_80

num_to_ip()
{
    local test_num=$1
    local test_net=$2

    if [[ ${test_net} =~ '.' ]];then
        local part_3=$[test_num/256+1]
        local part_4=$[test_num%256]
        echo "${test_net}.${part_3}.${part_4}"
    else
        local part_3=`printf "%x" $[test_num/65536+1]`
        local part_4=`printf "%x" $[test_num%65536]`
        echo "${test_net}::${part_3}:${part_4}"
    fi
}

ip_to_num()
{
    local test_ip=$1

    local part_3=`echo ${test_ip} | awk -F '[.|:]' '{print $(NF-1)}'`
    local part_4=`echo ${test_ip} | awk -F '[.|:]' '{print $NF}'`

    if [[ ${test_ip} =~ : ]];then
        part_3=`printf "%d" 0x${part_3}`
        part_4=`printf "%d" 0x${part_4}`
        echo "(${part_3}-1)*65536+${part_4}" | bc
    else
        echo "(${part_3}-1)*256+${part_4}" | bc
    fi
}


ak_read_key()
{
    local tmp_key=${1};shift

    [[ "${OTHER_ARGS}" =~ "${tmp_key}" ]] && echo "${tmp_key}" && return 0
    echo "#${tmp_key}" && return 1
}
ak_read_pair()
{
    local tmp_key=${1};shift   
    local def_val=${1};shift   

    for tmp_pair in ${OTHER_ARGS}
    do
        ## explicitly assigned
        [[ "${tmp_pair}" =~ "${tmp_key}=" ]] && echo "${tmp_pair//=/ }" && return 0
    done

    ## not explicitly assigned but has default value
    [ ! -z ${def_val} ] && echo "${tmp_key} ${def_val}" && return 0

    ## not explicitly assigned and no default value
    echo "#${tmp_key} XXX" && return 1
}

ak_add_checker()
{
    local hc_rip=${1};shift

    local hc_args="connect_port ${HC_PORT}
            `ak_read_pair connect_timeout $[RANDOM%1000+1]`
            `ak_read_pair delay_before_retry $[RANDOM%10+1]`
            `ak_read_pair retry $[RANDOM%3+1]`
            `ak_read_pair bindto`"

    case ${HC_TYPE} in
    tcp)
        echo "TCP_CHECK {
            ${hc_args}
        }"
        ;;
    http)
        echo "HTTP_GET {
            url {
                path /
                status_code 200
            }
            ${hc_args}
        }"
        ;;
    ssl)
        echo "SSL_GET {
            url {
                path /
                status_code 200
            }
            ${hc_args}
        }"
        ;;
    misc)
        echo "MISC_CHECK {
            misc_path \"/root/misc_check.sh ${hc_rip} ${HC_PORT} 10 5 3 3 > /dev/null && exit 0 || exit 1\"
            misc_timeout 15
            misc_dynamic
        }"
        ;;
    esac
}
ak_add_rs()
{
    local rip=${1};shift
    echo "
    real_server ${rip} ${RIP_PORT} {
        `ak_read_pair weight $[RANDOM%10+1]`
        `ak_add_checker ${rip}`
    }"
}

ak_add_vs()
{
    local vip=${1};shift

    [ -f /run/dpvs.pid ] && ip_tool=/usr/bin/dpip || ip_tool=/usr/sbin/ip
    list_algo=(rr wrr lc wlc sh consh)
    list_algo_len=${#list_algo[@]}

    echo "
virtual_server ${vip} ${VIP_PORT} {
    protocol ${PROTO}
    lb_kind ${LB_KIND}
    lb_algo rr
    `ak_read_pair delay_loop $[RANDOM%10+1]`
    `ak_read_pair persistence_timeout $[RANDOM%120+1]`
    alpha
    hysteresis 0
    quorum 1
    quorum_up \"${ip_tool} addr add ${vip} `ak_read_pair dev lo`\"
    #quorum_down \"${ip_tool} addr del ${vip} `ak_read_pair dev lo`\"
    $(for nr in `seq ${RIP_START} ${RIP_END}`;do
        rip=`num_to_ip ${nr} ${RIP_NET}`
            ak_add_rs ${rip}
    done)
}"
}

ak_sts_conf()
{
    mkdir -p ${CONF_DIR}
    for nv in `seq ${VIP_START} ${VIP_END}`;do
        vip=`num_to_ip ${nv} ${VIP_NET}`
        tmp_conf=${CONF_DIR}/`printf %.5d ${nv}`_${PROTO}_${vip}_${VIP_PORT}_${LB_KIND}_${HC_TYPE}_${HC_PORT}.conf
        ak_add_vs ${vip} > ${tmp_conf} &
    done
    wait
}


## default args
CONF_DIR=sts

VIP_START=1
VIP_END=1
VIP_NET=44
VIP_PORT=22

PROTO=TCP
LB_KIND=DR
LG_NAME=lg_v6
CIP_NET=22

RIP_START=1
RIP_END=1
RIP_NET=33
RIP_PORT=22

HC_TYPE=tcp
HC_PORT=22
## update args by input
while true; do
    case "$1" in
        -d | --dir )
            CONF_DIR=$2
            shift 2 ;;
        -v | --vip_port_num )
            arr=(${2//_/ })
            vip=${arr[0]}
            if [[ ${vip} =~ : ]];then
                VIP_NET=${vip%::*}
            else
                VIP_NET=${vip%.*.*}
            fi
            VIP_PORT=${arr[1]}
            VIP_END=${arr[2]}
            VIP_START=`ip_to_num ${vip}`
            VIP_END=$[VIP_START+VIP_END-1]
            shift 2 ;;
        -r | --rip_port_num )
            arr=(${2//_/ })
            rip=${arr[0]}
            if [[ ${rip} =~ : ]];then
                RIP_NET=${rip%::*}
            else
                RIP_NET=${rip%.*.*}
            fi
            RIP_PORT=${arr[1]}
            RIP_END=${arr[2]}
            RIP_START=`ip_to_num ${rip}`
            RIP_END=$[RIP_START+RIP_END-1]
            shift 2 ;;
        -h | --hck_port )
            arr=(${2//_/ })
            HC_TYPE=${arr[0]}
            HC_PORT=${arr[1]}
            shift 2 ;;
        -p | --protocol )
            PROTO=$2
            shift 2 ;;
        -k | --lb_kind )
            LB_KIND=$2
            shift 2 ;;
        * )
            break ;;
    esac
done
OTHER_ARGS="$*"
# OTHER_ARGS="$* alpha omega dev=eth1 bindto=33::5"

ak_sts_conf
