echo "Usage: $0 <blocksize> {read|randread|write|randwrite} [<sessions>]"

BLOCKSIZE="$1"
ACTION="$2"
SESSIONS=$3
JOBS=15

if [ ! $DURATION ]; then
        DURATION=120
fi

CWD=`pwd`

if [ ! $SESSIONS ]; then
	SESSIONS=15
fi

TESTDESC=$BLOCKSIZE"_"$ACTION"_"$SESSIONS"fioX"$JOBS"jobs_"$DURATION"sec"
stamp=`date +%Y%m%dT%H%M%S`
# echo -e "\n$stamp $TESTDESC, cleaning up the previous run $CWD/$TESTDESC-*.txt"
rm -rf "$CWD/$TESTDESC-*.txt"

echo  -e "$stamp test $TESTDESC starts"| tee -a $CWD/fiotest.log
for ((i=1; i<=$SESSIONS; i++)); do
	rptfile="./$TESTDESC-$i.txt"
	/usr/bin/fio -filename=/mnt/fuse/BBB/testB1_$i -direct=1 -thread -rw=$ACTION -ioengine=psync -bs=$BLOCKSIZE -size=5G -numjobs=$JOBS -runtime=$DURATION -group_reporting -name=fiotest > $rptfile &
done

RUNINGS=$SESSIONS
tmp=0
while [ $RUNINGS -gt 0 ]; do 
	sleep 1; 
	RUNINGS=`ps aux|grep '/usr/bin/fio'|grep 'fiotest' |wc -l`
	echo -ne "\r  $TESTDESC >> $RUNINGS fio are running "
	let "tmp += 1"; let "tmp %= 5"; for ((i=0; i<= $tmp; i++)) do echo -n '.'; done;
	echo -ne '        \r';
done

sum=0
max=0
min=9999999
for i in `grep iops ./$TESTDESC-*.txt |sed 's/.*iops=\([0-9]*\),.*/\1/g'`; do 
	let "sum += i";
	if [ $max -lt $i ]; then
		let "max = $i"
	fi
	if [ $min -gt $i ]; then
		let "min = $i"
	fi
done; 

stamp=`date +%Y%m%dT%H%M%S`

rm -f cdmi.0.log cdmifuse.conf cdmi.txt cdmi_stat.txt readhit.txt

cp -f /opt/cdmifuse/logs/cdmi.0.log .
cp -f /opt/cdmifuse/etc/cdmifuse.conf .

OTHERSTAT=""
echo -ne "\n  stat. log data ..."
grep 'nonCdmi_Update\|nonCdmi_Read' /opt/cdmifuse/logs/cdmi.2.log /opt/cdmifuse/logs/cdmi.1.log cdmi.0.log | grep -v DEBUG |grep took|sed -e 's/cdmi.*log://g' > cdmi.txt
TIMEWIN=`head -1 cdmi.txt |cut -f 2 -d ' '`~`tail -1 cdmi.txt  |cut -f 2 -d ' '`

if [ $ACTION == "read" ] || [ $ACTION == "randread" ]; then 
	grep 'nonCdmi_ReadData' cdmi.txt | grep ' INFO ' | sed -e 's/.*took \([0-9]*\)ms.*len(\([0-9]*\).*/\2 \1/g'  > cdmi_stat.txt
	grep 'hit' /opt/cdmifuse/logs/cdmi.1.log cdmi.0.log | sed -e 's/.*hit\[\([0-9]*\).*miss\[\([0-9]*\).*readAhead\[\([0-9]*\).*/\1 \2 \3/g' > readhit.txt
	OTHERSTAT=`awk '{c+=1;t+=$1+$2;h+=$1} END { print "cache-hitrate=" h/t*100 "%"; }' readhit.txt`
else
	grep 'nonCdmi_UpdateData' cdmi.txt | grep ' INFO ' | sed -e 's/.*totalLen\[\([0-9]*\)\].*took \([0-9]*\)ms.*/\1 \2/g' > cdmi_stat.txt
fi
echo ""

CDMI_STAT=`awk '{if(min1==""){min1=max1=$1}; if(min2==""){min2=max2=$2}; if($1>max1){max1=$1}; if($2>max2){max2=$2};if($1< min1){min1=$1}; if($2< min2){min2=$2}; total1+=$1;total2+=$2; count+=1} END {print "cdmi-count=" count " length-mean=" total1/count " in[" min1 "~" max1 "] cdmi-latency-mean=" total2/count "ms in[" min2 "~" max2 "]";}' cdmi_stat.txt`
# rm -f cdmi.txt cdmi_stat.txt readhit.txt
 
tar cfv - cdmifuse.conf cdmi.0.log cdmi.txt $TESTDESC-*.txt | gzip -9 - > $TESTDESC-$stamp-$sum.tar.gz
rm -f $CWD/$TESTDESC-*.txt

echo -e "\r$stamp $TESTDESC done, iops=$sum in[$min~$max]x$SESSIONS; SampleTimeWin[$TIMEWIN]-Stat: $CDMI_STAT; $OTHERSTAT" | tee -a $CWD/fiotest.log

