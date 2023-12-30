#!/bin/bash

usage(){
echo 
echo	"TsStat.sh Utilty to collect TianShan log files"
echo   	"Usage: TsStat [-d MM-DD-YYYY] [-h]"
echo	"	-d  MM-DD-YYYY   to specify the date from when the utility should collect the log files"
echo	"	-h               show help"
echo	"CurrentSettings:"
echo    "	WORKDIR      = "${WORKDIR}
echo    "	TianShanHome = "${TianShanHome}
echo    "	TianShanLogs = "${TianShanLogs}
echo    "	TianShanEtc  = "${TianShanEtc} 
echo	"Please edit the batch job to correct the settings if the above didn't match"
echo
}


WORKDIR=/home/yixin.tian/work
TianShanHome=/opt/TianShan
TianShanLogs=${TianShanHome}/logs
TianShanEtc=${TianShanHome}/etc
ZIPCMD="tar -czf"

LogSince=

usage

while getopts "hd:" opt; do
    case ${opt} in
    h) exit 0 ;;
    d) LogSince=${OPTARG} ;;
    \?) echo "invalid parameters";  exit 1 ;;
    esac
done


#parse log since date 
if [ -n "${LogSince}" ]; then
	month=${LogSince:0:2}
#	echo ${month}
	if [ ${month} -ge 1 ] && [ ${month} -le 12 ];then
		:	
	else
		echo "invalid parameter:"${LogSince}
		exit 0
	fi

	day=${LogSince:3:2}
#	echo ${day}
	if [ ${day} -ge 1 ] && [ ${month} -le 31 ];then
		:
	else
		echo "invalid parameter:"${LogSince}
		exit 0
	fi

	year=${LogSince:6:4}
#	echo ${year}
	if [ ${year} -ge 0 ] && [ ${month} -le 9999 ];then
		:
	else
		echo "invalid parameter:"${LogSince}
		exit 0
	fi

	LogSince=${year}-${month}-${day}
#	echo ${LogSince};
fi


HOSTNAME=$(hostname)
if [ -z ${HOSTNAME} ];then
	HOSTNAME=TsStat_collector
fi
echo ${HOSTNAME}

cd  ${WORKDIR}
if [ $? -ne 0 ];then
	echo "fialed go to dir: "${WORKDIR}
	exit 1;
fi

rm -fr ${HOSTNAME}
if [ $? -ne 0 ];then
	echo "fialed remove dir: "${HOSTNAME}
	exit 1;
fi

mkdir ${HOSTNAME}
if [ $? -ne 0 ];then
	echo "fialed make dir: "${HOSTNAME}
	exit 1;
fi

cd  ${HOSTNAME}


echo collecting Service configuration item...
cp /etc/TianShan.xml ./ >copy.log 2>&1

echo collecting process information ...
top -b -n 1 >proc.txt

echo collecting net stat ...
netstat -a -n -p >netstat.txt

echo "collecting configuration from "${TianShanEtc} ...
cp -rv ${TianShanEtc} ./ >>copy.log 2>&1

echo "collecting log files from "${TianShanLogs} ...
if [ -z  ${LogSince} ];then
	cp -rv  ${TianShanLogs} ./ >>copy.log 2>&1
else
	files=$(ls ${TianShanLogs})
	subdir=${TianShanLogs##/*/}
	if [ -z ${subdir} ];then
		subdir=${TianShanLogs%"/"}
		subdir=${subdir##/*/}
	fi

	mkdir ${subdir}

	for file in ${files};do
#		echo ${file}
		filedate=$(stat --printf="%y" ${TianShanLogs}/${file})
		modifytime=${filedate:0:10}

		if [[ ${modifytime} > ${LogSince}  ]] || [ ${modifytime} == ${LogSince} ];then
			cp -rv ${TianShanLogs}/${file} ./${subdir}/ >>copy.log 2>&1
		fi
	done
fi


ZIPNAME=TsStat_${HOSTNAME}_$(date +%Y.%m.%d-%H.%M.%S).tar.gz
echo ${ZIPNAME}

echo "creating log archive "${ZIPNAME} ...
cd ..

${ZIPCMD} ${ZIPNAME} ${HOSTNAME} 
rm -rf  ${HOSTNAME}







