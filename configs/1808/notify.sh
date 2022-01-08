#!/bin/sh

FILE=/var/run/ha-groups/
STATE="$3"

mkdir -p $(dirname "$FILE")
case "$STATE" in
	"MASTER")
		echo "master" > "$FILE"
		;;
	*)
		echo "backup" > "$FILE"
		;;
esac
