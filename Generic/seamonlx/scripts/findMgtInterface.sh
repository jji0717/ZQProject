#!/bin/bash

NETWORKCFG_DIR=/etc/sysconfig/network-scripts
array=(`ls ${NETWORKCFG_DIR}/ifcfg-eth*`)
count=${#array[*]}

declare -a interfaces

j=0
i=0
while [ $i -lt $count ]; do
	file=${array[$i]}
	cat $file | grep BOOTPROTO=dhcp > /dev/null 2>&1
 	if [ $? -eq 0 ]; then
 		interfaces[$j]=${file##/etc*ifcfg-}
		let j++
 	fi
	let i++
done 

for ((idx=0;idx<$j;idx++)); do
	if [ $idx -gt 0 ]; then
		echo -n ", "
	fi
	echo -n "${interfaces[$idx]}"
done
echo