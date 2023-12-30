#!/bin/bash 
#
# Copyright (c) 2009, SeaChange International
# All Rights Reserved
# Get the list of external packages
#
#
# grep the cfgfile, pipe output to awk, find second field, pipe that to perl to create a string
# which is what the user wants
cfgfile=/usr/local/seachange/configdata/config.dat
if [ -f $cfgfile ]; then
	plist=`grep "^seamonlx.monitor.rpm" "$cfgfile"  | awk -F '=' '{print $2}' | perl -p -e 's/[\r\n\s]/ /g'`
fi

cfgfile1=/usr/local/seamonlx/config/seamonlx.conf
if [ -f $cfgfile1 ]; then
	plist1=`grep "^seamonlx.monitor.rpm" "$cfgfile1"  | awk -F '=' '{print $2}' | perl -p -e 's/[\r\n\s]/ /g'`
	echo "$plist $plist1"
	exit 0
fi
exit 1

