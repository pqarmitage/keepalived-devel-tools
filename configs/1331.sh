#!/bin/bash

cat <<EOF
global_defs {                                
   router_id keepalived                      
   vrrp_mcast_group4 224.0.0.118             
}
EOF


for i in $(seq 1 100); do
	cat <<EOF

vrrp_script chk_$i {
#    script "/etc/keepalived/scripts/check.sh"
    script /home/quentin/keepalived/utils/configs/scripts/vs.sh
    interval 10
    timeout 10
}

vrrp_instance redis_$i {                  
        state BACKUP                         
        interface eth0                      
        virtual_router_id $i                 
        priority 120                         
        preempt
        advert_int 1                         
        authentication {                     
            auth_type PASS                   
            auth_pass 1111                   
        }                                    
        virtual_ipaddress { 
	    10.25.88.$((i + 49))
        }                                    
        track_script {                       
            chk_$i                        
        }                                    
}
EOF
done
