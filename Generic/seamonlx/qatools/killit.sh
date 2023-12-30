#!/bin/bash
#
# sample of cmd line execution:
#
# $0 = script name 
# $1 = name of proc to find pid for

proclist=`pidof $1`

for procpid in $proclist
do
	kill -9 $procpid
done
