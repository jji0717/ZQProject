#!/bin/bash

ROOT_UID=0
DATESTAMP=`date +%Y%2m%2d-%H%m%S`
LOGDIR='/tmp/seamonlx-seastats'
LOGFILE='seamonlx-'${HOSTNAME}


#########################################################################
#
# RunningAsRoot
#
# Make sure we are running as root
#
#########################################################################

function RunningAsRoot
{
    if [ "${UID}" -ne  "${ROOT_UID}" ]
    then
	echo "Must be root to run this script."
	exit $E_NOTROOT
    fi 
}


########################################################################
#
# printSeaMonLXConfig
#
########################################################################

function printSeaMonLXConfig
{
    echo "Collecting SeaMon LX config files"
    mkdir -p ${LOGDIR}/config

    [ -f /usr/local/seamonlx/config/seamonlx.conf ] && cp /usr/local/seamonlx/config/seamonlx.conf ${LOGDIR}/config/seamonlx.conf 2>&1
    [ -f /usr/local/seamonlx/config/StringResource.ini ] && cp /usr/local/seamonlx/config/StringResource.ini ${LOGDIR}/config/StringResource.ini 2>&1
}

########################################################################
#
# printSeaMonLXLog
#
########################################################################

function printSeaMonLXLog
{
    echo "Collecting SeaMon LX log files"
    mkdir -p ${LOGDIR}/logs

    #[ -f /var/www/seaviewlx/log/development.log ] && cp /var/www/seaviewlx/log/development.log ${LOGDIR}/logs/development.log 2>&1
}

########################################################################
#
# printSeaMonLXXML
#
########################################################################

function printSeaMonLXXML
{
    echo "Collecting SeaMon LX XML files"
    mkdir -p ${LOGDIR}/data

    #[ -f xxx/var/www/seaviewlx/log/development.log ] && cp xxx/var/www/seaviewlx/log/development.log ${LOGDIR}/data/.xml 2>&1
}

#
# Check to see if we are running as root
#

RunningAsRoot

#
# Create directory if needed
#
mkdir -p ${LOGDIR}
rm -f ${LOGDIR}.zip

( date; uname -a ) >${LOGDIR}/${LOGFILE}

#
# Dump information to our info file
#
echo "Hostname is : ${HOSTNAME}" >${LOGDIR}/${LOGFILE}
echo "Date is     : `date`" >>${LOGDIR}/${LOGFILE}
uname -a >> ${LOGDIR}/${LOGFILE} 

#
# Report SeaMon LX configuration
#
printSeaMonLXConfig

#
# Report SeaMon LX Logs
#
printSeaMonLXLog

#
# Report SeaMon LX XML data files
# for Topology, Health, SeaStats, etc.
#
printSeaMonLXXML

#
# Have all of the files we want, generate the zip file!
#

pushd ${LOGDIR} >/dev/null
/usr/bin/zip -r -m ../seamonlx-seastats.zip * >/dev/null
popd >/dev/null
rmdir ${LOGDIR}
exit 0
