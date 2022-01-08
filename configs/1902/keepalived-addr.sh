#!/bin/bash
set -eux -o pipefail

# The IP addresses are compared so that the controller with the highest IP will
# be given higher VRRP priority ensuring that there is always a winner in
# elections.

INTERFACE="$(ls /var/config/ha/vrrp/interface/ | tail -n1)"

# Ask all IPv6 nodes on the network to reply so they will appear in the
# neighbours table
ping -i0.2 -I $INTERFACE -c2 ff02::1 || true

# If there is a reachable leader then fail this script
if ip -6 neigh show dev $INTERFACE nud reachable | grep -q 1ead; then
  exit 1
fi

IP="$(ip -6 addr show dev $INTERFACE scope link -nodad | sed -n 's/^.*inet6 \(.*\)\/.*$/\1/p')"

if [[ "$(printf "$(ip -6 neigh show dev $INTERFACE | sed -n 's/^\(.*\) lladdr.*REACHABLE$/\1/p')\n$IP" | sort | tail -n1)" == "$IP" ]]; then
  exit 0
fi

exit 1
