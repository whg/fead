#!/bin/bash

uid=$1
output="uid-eeprom.hex"

if [[ -z $uid ]]; then
	echo "usage: $0 <uid>"
	exit 1
fi


checksum=$(((0xff ^ ((31 * 0xff + 0x20 + 0x00 + 0xc0 + 0x00 + $uid) & 0xff)) + 1))

printf ":2000C000FFFFFFFFFFFFFFFF%02XFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF%02X\n" \
	   $uid $checksum >$output

echo ":00000001FF" >>$output

echo "saved $output"
