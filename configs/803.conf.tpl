@low net_namespace low
@high net_namespace high

$VI=\
vrrp_instance 4${NUM} {  \
    state BACKUP \
    interface bond0.${NUM} \
    use_vmac vrrp4.${NUM}.1 \
    virtual_router_id 1 \
@high    priority 115 \
@low     priority 100 \
    advert_int 1 \
    preempt_delay 60 \
    authentication { \
        auth_type PASS \
        auth_pass a3dswd3410 \
    } \
    virtual_ipaddress {  \
        172.16.${NUM}.254/24 \
    } \
    mcast_src_ip 172.16.${NUM}.254 \
}

