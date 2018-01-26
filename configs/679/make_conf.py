#!/usr/bin/env python3

import jinja2

class Vlan:
    def __init__(self, name, ip4addr, ip4net, ip6addr):
        self.name = name
        self.ip4addr = ip4addr
        self.ip4net = ip4net
        self.ip6addr = ip6addr

vlans = []
for looper in range(0,1):
    for vlan_id in range(2,100):
        looper_vlan = ( looper * 256 ) + vlan_id
        ip4addr = "10." + str(looper) + "." + str(vlan_id) + ".254/24"
        ip4net = "10." + str(looper) + "." + str(vlan_id) + ".0/24"
        ip6addr = "2001:41d0:ffff:" + format(looper_vlan, '02x') + ":ff:ff:ff:ff/64"
        new_vlan = Vlan(looper_vlan, ip4addr, ip4net, ip6addr) 
        vlans.append(new_vlan)


to_return = ""
templateLoader = jinja2.FileSystemLoader( searchpath="./" )
templateEnv = jinja2.Environment( loader=templateLoader )
vrrpconf = templateEnv.get_template('template_vrrp_instance').render(vlans=vlans) + "\n"

output_file = open("conf_vrrp_full", "w")
output_file.write(vrrpconf)
output_file.close()
