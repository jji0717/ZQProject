Name:      TianShan
Version:   2.0.0
Release:   1
Summary:   XORMedia CacheEngine
Group:     Applications/System
License:   commercial
BuildArch: x86_64
BuildRoot: %{_tmppath}/%{name}-root

Requires: python >= 2.4
Requires: libicu >= 4.2.1
Requires: boost-regex >= 1.41.0
Requires: libaio
#Requires: graphviz

%define __os_install_post  %{nil}
%define __prelink_undo_cmd %{nil}

%define _topdir       %(echo ${HOME})
%define _rpmdir       %(echo ${OutputPath})
%define _rpmfilename  %(echo ${Package})
%define _builddir     ${CacheManagement}/build

%define prefix        /opt/%{name}
%define postfix       %([ $(uname -m) = "x86_64" ] && echo "64")
%define oid           .1.3.6.1.4.1.22839.4.1
%define snmp_agent    %{prefix}/bin/libSnmpAgent.so
%define snmp_server   127.0.0.1

%define sdk_dir       $([ -z "${ZQSdkPath}" ] && echo "/opt/sdk" || echo ${ZQSdkPath})
%define ice_dir       %{sdk_dir}/3rdPartyKits/ice
%define boost_dir     %{sdk_dir}/3rdPartyKits/boost
%define snmp_dir      %{sdk_dir}/3rdPartyKits/net-snmp
%define snmpp_dir     %{sdk_dir}/3rdPartyKits/snmp++
%define libnl_dir     %{sdk_dir}/3rdPartyKits/libnl
%define libpcap_dir   %{sdk_dir}/3rdPartyKits/libpcap
%define e2fsprogs_dir %{sdk_dir}/3rdPartyKits/e2fsprogs
%define curl_dir	    %{sdk_dir}/3rdPartyKits/curl

%define tssdk_dir     %{sdk_dir}/TianShanSDK

%define src_dir       ${CacheManagement}
%define src_cfg_dir   %{src_dir}/etc
%define src_bin_dir   %{src_dir}/bin%{postfix}
%define src_lib_dir   %{src_dir}/lib%{postfix}
%define src_build_dir %{_builddir}
%define src_util_dir  %{src_build_dir}/utils
%define src_mod_dir   %{src_dir}/modules

%define dest_dir      %{buildroot}/%{prefix}
%define dest_cfg_dir  %{dest_dir}/etc
%define dest_bin_dir  %{dest_dir}/bin
%define dest_util_dir %{dest_dir}/utils
%define dest_mod_dir  %{dest_dir}/modules
%define dest_svc_dir  %{buildroot}/etc/init.d

%define svc_daemons   CMEV2Svc

# NSS
%define svc_runlevel  345
%define svc_seq_pre   9

%define dep_ice       Ice IceUtil IceStorm IceBox IceStormService IceGrid Glacier2 Freeze
%define dep_misc      snmp snmp++ db_cxx boost_regex icu nl

%define ice_ver       3.2.2
%define ice_ver_maj   32
%define boost_ver     1.33.1
%define boost_ver_maj 2
%define snmp_ver      10.0.3
%define snmp_ver_maj  10
%define snmpp_ver     3.2.24
%define snmpp_ver_maj 3
%define db_cxx_ver    4.5
%define icu_ver       36.0
%define icu_ver_maj   36
%define nl_ver        1.1
%define nl_ver_maj    1
%define curl_ver      4.3.0
%define curl_ver_maj  4
%define pcap_ver      1.3.0
%define pcap_ver_maj  1
%define uuid_ver_maj  1
%define uuid_ver      1.2
 
AutoReqProv: no

%description
ZQ Stream Server

%prep

# clean object files and target files in sub dirs
make veryclean

# clean temp dir for rpm package
[ -d %{buildroot} ] && rm -rvf %{buildroot}

%build

make debug=${Debug} version=%{version}.%{release}

%pre

# stop all running service before upgrade
if [ "$1" = "2" ]; then 
    [ -x %{prefix}/bin/SystemShell ] && %{prefix}/bin/SystemShell -s CMEV2Svc
    exit 0
fi

rpm -q net-snmp >/dev/null
if [ $? -ne 0 ]; then
    echo "WARNING: package net-snmp is not installed, some of the components will not function."  
    while true; do
        echo -n "continue? [yes|no]: "
        exec 0</dev/tty
        read answer
        
        if [ "${answer}" != "yes" -a "${answer}" != "no" ]; then
            echo "please enter \"yes\" or \"no\""
            continue
        elif [ "${answer}" = "no" ]; then
            exit 1
        else
            break
        fi
    done
else
    service snmpd stop >/dev/null
fi

%install 

mkdir -pv %{buildroot}/{etc/init.d,opt/%{name}/{bin,logs/{crashdump,rtrec},etc,data/runtime,utils,modules}}
mkdir -pv %{src_mod_dir}

# utils
chmod a+x %{src_util_dir}/ver_check 
cp -vp %{src_util_dir}/ver_check %{dest_util_dir}

# service scripts

CMEV2Svc="CMEV2 is designed to mamage the CDN content caching on FMS video servers.\n\
#              It receives session nofication from OSTR to decide the content popularity,\n\
#              and then interface with VSIS service on FMS to import or delete content"


i=0
for svc in %{svc_daemons}; do
    script=$(echo ${svc} | sed 's/.*/\L&\E/')
    eval desc=\$${svc}
    echo -e "#!/bin/bash\n#\n# chkconfig: %{svc_runlevel} %{svc_seq_pre}${i} 04\n# description: ${desc}\n#\n\nservice=${svc}\n\n. %{prefix}/utils/tianshan4cme\n" > %{src_util_dir}/${script}
    chmod a+x %{src_util_dir}/${script}
    cp -vp %{src_util_dir}/${script} %{dest_svc_dir}
#	i=$((i%10)); i=$((i+1))
	[ $i -lt 9 ] && i=$((i+1))
done
chmod a+x %{src_util_dir}/tianshan4cme
cp -vp %{src_util_dir}/tianshan4cme %{dest_util_dir}
dos2unix %{dest_util_dir}/tianshan4cme

# all exec and shared libraries
cp -vrP %{src_bin_dir}/* %{dest_bin_dir}

# TianShanSDK library
cp -vP \
%{tssdk_dir}/lib/{libSnmpManPkg.so,libSnmpManPkg.so.*} \
%{tssdk_dir}/lib/{libSysLogger.so,libSysLogger.so.*} \
%{tssdk_dir}/lib/{libZQCommon.so,libZQCommon.so*} \
%{tssdk_dir}/lib/{SystemShell,SystemShell.*} \
%{dest_bin_dir}

# third party libraries

# misc
cp -vP \
%{snmp_dir}/lib/{libsnmp.so.%{snmp_ver},libsnmp.so.%{snmp_ver_maj}} \
%{snmpp_dir}/lib/{libsnmp++.so.%{snmpp_ver},libsnmp++.so.%{snmpp_ver_maj}} \
%{libnl_dir}/lib/{libnl.so.%{nl_ver},libnl.so.%{nl_ver_maj}} \
%{e2fsprogs_dir}/lib/{libuuid.so,libuuid.so.%{uuid_ver},libuuid.so.%{uuid_ver_maj}} \
%{curl_dir}/lib/{libcurl.so.%{curl_ver},libcurl.so.%{curl_ver_maj}} \
%{dest_bin_dir}

#%{_libdir}/{libicui18n.so.%{icu_ver},libicui18n.so.%{icu_ver_maj}} \
#%{_libdir}/{libicuuc.so.%{icu_ver},libicuuc.so.%{icu_ver_maj}} \
#%{_libdir}/{libicudata.so.%{icu_ver},libicudata.so.%{icu_ver_maj}} \
#%{boost_dir}/lib/{libboost_regex.so.%{boost_ver},libboost_regex.so.%{boost_ver_maj}} \


# sample configuration files
sed -i '{ 
    s/\(.*TianShanHomeDir.*value.*=\).*\(\/>\)/\1"\/opt\/%{name}"\2/
}' %{src_cfg_dir}/TianShanDef.xml


for file in %{src_cfg_dir}/*.xml; do
    base=$(basename ${file})
	
    if [ "${base%_linux.xml}" = "${base}" ]; then
        # xxx.xml do not touch files below
        if [ "${base}" = "TianShan4CME.xml" ]; then
            dest=${base}
        else
            dest=${base%.xml}_sample.xml
        fi
    else
        # xxx_linux.xml
        dest=${base%_linux.xml}_sample.xml
    fi

	case "${base}" in
	"syntax.xml"|"CRM_C2Locator.xml"|"EGH_JMS.xml"|"MovieOnDemand.xml") ;;
	*) sed -i 's/\\/\//g' ${file} ;;
	esac

    cp -v ${file} %{dest_cfg_dir}/${dest}
done

%post

# to avoid conflict with TianShan services
if [ ! -e /etc/TianShan.xml ]; then
	ln -sf %{prefix}/etc/TianShan4CME.xml /etc/TianShan.xml
fi

# verify daemon scripts
for svc in %{svc_daemons}; do
	script=$(echo ${svc} | sed 's/.*/\L&\E/')
	# clean the startup script first if this is upgrade
	if [ "$1" == 2 ]; then
		/sbin/chkconfig --del ${script}
	fi
	/sbin/chkconfig --add ${script}
done

# verify snmpd service
rpm -q net-snmp >/dev/null

if [ $? -eq 0 ]; then
    snmpcfg=/etc/snmp/snmpd.conf

    # auto generate configuration 
    community=$(egrep "[[:space:]]*rwcommunity[[:space:]]+%{name}" ${snmpcfg})
    if [ -z "${community}" ]; then
        echo "rwcommunity %{name} %{snmp_server} %{oid}" >> ${snmpcfg}
    fi 
    agent=$(egrep "[[:space:]]*dlmod[[:space:]]+SNMPAgent[[:space:]]+.*libSnmpAgent.so" ${snmpcfg})
    if [ -z "${agent}" ]; then  
        echo "dlmod SNMPAgent %{snmp_agent}" >> ${snmpcfg}
    fi

    # make snmpd start with system
    dir=/etc/rc2.d
    file=$(find ${dir}/* -type l -name "[KS]??snmpd")

    # this should always be true if net-snmp installed
    if [ -n "${file}" ]; then
        f=$(basename ${file})
        if [ "${f:0:1}" = "K" ]; then
            mv ${dir}/{K,S}${f:1}
        fi
    else
        # not possible here though, just in case
        ln -s ../init.d/snmpd ${dir}/S50snmpd
    fi  
    
    stat /var/run/snmpd.pid >/dev/null 2>&1
    if [ $? -ne 0 ]; then
        service snmpd start >/dev/null
    fi
fi

%preun

[ -x %{prefix}/bin/SystemShell ] && %{prefix}/bin/SystemShell -s CMEV2Svc

# delete chkconfig settings
for svc in %{svc_daemons}; do
	script=$(echo ${svc} | sed 's/.*/\L&\E/')
	/sbin/chkconfig --del ${script}
done

%postun

# do nothing for an upgrade
[ "$1" = "1" ] && exit 0

# restore default config for core dumps
echo 'core' > /proc/sys/kernel/core_pattern

#to avoid delete TianShan's xml file
#rm -f /etc/TianShan.xml

%clean

cd %{buildroot}
name=${Package%.*}
zip -rvy %{_rpmdir}/${name}.zip *
cd -

# make package for debug symbols
cd ${CacheManagement}/symbols
zip -rv %{_rpmdir}/${Symbols} *
cd -

rm -rf %{buildroot}

%files 
%defattr(-,root,root)

%config %attr(0555,root,root) %{prefix}/etc/ 

%{prefix}/logs/
%{prefix}/data/
%{prefix}/utils/
%{prefix}/modules/
%{prefix}/bin/
/etc/init.d/*

# vim: ts=4 sw=4 bg=dark

