#!/bin/bash

become_leader() {
  until redis-cli REPLICAOF NO ONE
  do
    echo "Attempting to make node leader failed trying again in 500ms"
    sleep 0.5
  done
}

become_replica() {
  until redis-cli REPLICAOF <ADDRESS> 6379
  do
    echo "Attempting to set replication failed trying again in 500ms"
    sleep 0.5
  done
}

case $3 in
  "MASTER")
    become_leader
    exit 0
  ;;
  "BACKUP")
    become_replica
    exit 0
  ;;
  "FAULT")
    # The safest thing to do is to attempt to become a replica. Either redis is
    # running and replication fails, but we have set the node as read only
    # prevent split brain, or redis is not running and this is does nothing.
    become_replica
    exit 0
  ;;
  *)
    echo "Expected MASTER, BACKUP, or FAULT"
    exit 1
  ;;
esac
