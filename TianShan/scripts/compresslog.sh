#!/bin/bash

LOGPATH=/opt/cdmifuse/logs/
LOGFNAME=cdmi

for ((i=20; i>3; i--)); do
	if [ ! -e $LOGPATH/$LOGFNAME.$i.log ]; then
		continue;
	fi
	
	stamp=`head -1 $LOGPATH/$LOGFNAME.$i.log | sed -e 's/\([^\-]*\)\-\([0-9]*\) \([0-9]*\):\([0-9]*\):\([0-9]*\).*/\1\2\3\4\5/g'`
	
	echo compressing $LOGPATH/$LOGFNAME.$i.log to $LOGPATH/$LOGFNAME.log.$stamp.bz2
	mv $LOGPATH/$LOGFNAME.$i.log $LOGPATH/$LOGFNAME.log.$stamp
	bzip2 $LOGPATH/$LOGFNAME.log.$stamp
done


