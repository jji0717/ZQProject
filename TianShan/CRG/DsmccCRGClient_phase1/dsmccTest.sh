#! /bin/bash

#ServerList format:("dsmccip1:port1,lscpip1:port1,MonitorIP,comment1"
#	       "dsmccip2:port2,lscpip2:port2,MonitorIP,comment2")
#如上需要检测的服务器列表格式，需修改对应的IP和port为Dsmcc服务器的IP和port,comment为需上报的固定信息，这里是

#DSMCC点流的虚拟IP和DSMCC点流的IP,每项中间以"|"分隔开

ServerList=(
	"10.15.10.73:9527,10.15.10.73:9528,MonitorIP,PublishedIP=10.15.10.74|internalIP=10.15.10.74",
	"10.15.10.73:9527,10.15.10.73:9528,MonitorIP,PublishedIP=10.15.10.73|internalIP=10.15.10.73"
)

TIANSHAN_HOME="/opt/TianShan"

EXE="$TIANSHAN_HOME/bin/DsmccCRGClient"
CONFIG="$TIANSHAN_HOME/etc/DsmccScript.xml"
LOGPATH="$TIANSHAN_HOME/logs/DsmccTest.log"

IFS=";"

for n in ${ServerList[@]}
do
	v1=`echo $n|awk -F ',' '{print $1}'`
	v2=`echo $n|awk -F ',' '{print $2}'`
	v3=`echo $n|awk -F ',' '{print $3}'`
	v4=`echo $n|awk -F ',' '{print $4}'`
	
	
	echo $EXE "-l" $LOGPATH "-c" $CONFIG "-d" $v1 "-p" $v2 "-i" $v3 "-m" $v4
	$EXE -l "$LOGPATH" -c "$CONFIG" -d "$v1" -p "$v2" -i "$v3" -m "$v4"
done
