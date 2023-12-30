#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#要求的autoconf的版本号
AC_PREREQ(2.59)

#AC_INIT(package, version, [bug-report], [tar-name])
AC_INIT(Big-TianShan, 1.0.0.0, [romalin99@163.com], [FunsLina8])
#AC_CONFIG_SUBDIRS(TianShan/configure.ac)

#指定辅助文件的目录；表示将configure生成的一些文件，放在这里（使得上层整洁些）
AC_CONFIG_AUX_DIR(config)

# get two environment variable
PROJSPATH=$ZQProjsPath
THIRDSDKPATH=$ZQSdkPath

clibv=".1.0.1.0"
celfv=".1.0.1.0"
#check Ice support
m4_include([m4/prebuild.m4])
m4_include([m4/icerule.m4])

AM_INIT_AUTOMAKE(Big-TianShan, 1.0.0.0)

#./configure --srcdir=DIR 指定源码目录
AC_CONFIG_SRCDIR([config])
#生成的配置头文件名为config.h
AC_CONFIG_HEADER([config.h])

AC_PROG_LIBTOOL
# Checks for programs.
CXXFLAGS=${CXXFLAGS:=-g}
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
#生成安装脚本 install-sh
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
#AC_PROG_RANLIB

#./configure --with_debug,自定义configure的参数
AC_ARG_ENABLE(debug,
				AS_HELP_STRING([--enable-debug],[Turn on debugging]),
				[case "${enableval}" in
				yes) debug=true;;
				no) debug=false;;
				*) AC_MSG_ERROR(bad value ${enableval} for  --enable-debug);;
				esac],[debug=false]
				)
AM_CONDITIONAL(DEBUG, test x$debug = xtrue )
AC_ARG_ENABLE(interinstall,
				AS_HELP_STRING([--enable-interinstall],[Turn on internal  install]),
				[case "${enableval}" in
					yes) interinstall=true;;
					no)  interinstall=false;;
					*)   AC_MSG_ERROR(bad value ${enableval} for --enable-interinstall);;
				 esac],
				[interinstall=true]
				)

AC_ARG_ENABLE(dbgfile,
				AS_HELP_STRING([--enable-dbgfile],[Turn on internal  install]),
				[case "${enableval}" in
					yes) dbgfile=true;;
					no)  dbgfile=false;;
					*)   AC_MSG_ERROR(bad value ${enableval} for --enable-dbgfile);;
				 esac],
				[dbgfile=true]
				)
AM_CONDITIONAL([INTERINSTALLFLAG], [test xtrue = xtrue])
AM_CONDITIONAL([PREBUILDFLAG], [test xtrue = xtrue])
AM_CONDITIONAL([DBGFILE], [test x$dbgfile = xtrue])
echo "#define VERSION \"${celfv/./}\"" > ./build/version.h

#指定默认安装目录，若不指定，则是/usr/local
AC_PREFIX_DEFAULT(/opt/TianShan)
AC_PREFIX_PROGRAM(HttpCRG)
# Checks for libraries.

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h memory.h netdb.h netinet/in.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/statvfs.h sys/time.h sys/timeb.h syslog.h unistd.h wchar.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_SUBST(clibv)
AC_SUBST(celfv)

#lib_versioninfo="-version-info 10:5:3"
#pro_versioninfo=".1.0.0.0"
#AC_SUBST(lib_versioninfo)
#AC_SUBST(pro_versioninfo)
AC_HEADER_STDBOOL
AC_DEFINE(VERSION_MAJOR, 0 ,[Major part of the version number])
AC_DEFINE(VERSION_MINOR, 1, [Minor part of the version number])

AC_DEFINE([__STRICT_ANSI__], 1, [__STRICT_ANSI__ Macro defined])
# configurable Ice locations

AC_C_CONST
AC_C_INLINE
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_C_VOLATILE

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_ERROR_AT_LINE
AC_REPLACE_FNMATCH
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRERROR_R
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([clock_gettime ftime ftruncate gethostbyaddr gethostbyname gethostname gettimeofday inet_ntoa localtime_r memset mkdir select socket strcasecmp strchr strdup strerror strrchr strstr strtol strtoul strtoull])

#AC_CONFIG_FILES()
AC_OUTPUT([Makefile 
Common/Makefile 
Common/HttpEngine/Makefile 
Common/DataPostHouse/Makefile 
Common/Rtsp/RtspParser/Makefile
TianShan/Makefile 
TianShan/Ice/Makefile 
TianShan/common/Makefile 
TianShan/common/snmp/Makefile 
TianShan/common/vrep/Makefile
TianShan/Shell/Makefile 
TianShan/Shell/ZQAppShell/Makefile 
TianShan/Shell/ZQSNMPManPkg/Makefile 
TianShan/Shell/ZQShell/Makefile 
TianShan/EventChannel/service/Makefile 
TianShan/CDNLib/Makefile 
TianShan/CDNLib/CRM_C2Locator/Makefile 
TianShan/CDNLib/Management/c2loc_web/Makefile
TianShan/CDNLib/CDNSS/C2Streamer/Makefile
TianShan/CDNLib/CDNSS/Makefile
TianShan/ContentStore/Makefile 
TianShan/ContentStore/BaseCS/Makefile
TianShan/ContentStore/ContentClient/Makefile 
TianShan/ContentStore/MediaClusterCS/Makefile
TianShan/CRG/Makefile 
TianShan/CRG/HttpCRG/Makefile 
TianShan/ComcastNGOD/Makefile 
TianShan/ComcastNGOD/CRM_A3Message/Makefile
TianShan/CPE/Makefile
TianShan/CPE/CPH_PCAP/Makefile
TianShan/CPE/CPH_C2Propagation/Makefile
TianShan/CPE/Pacing/Makefile
TianShan/CPE/CPH_CDN/Makefile
TianShan/CPE/CPH_RTI/Makefile
TianShan/CPE/service/Makefile
TianShan/CPE/CDNTrickGen/Makefile
TianShan/Sentry/Makefile
TianShan/Sentry/ZQSNMPExtAgent/Makefile
TianShan/Sentry/webLayout/Makefile
TianShan/Sentry/TSClient/Makefile
TianShan/Sentry/MsgSender/MsgSender/Makefile
Generic/Makefile
Generic/JndiClient/Makefile
TianShan/Sentry/Plug_EventSender/snmp/Makefile
TianShan/Sentry/Plug_EventSender/Makefile
TianShan/Sentry/Module/SysLogSender/Makefile
TianShan/Sentry/LogPage/Makefile
TianShan/Sentry/CPCMan_web/Makefile
TianShan/Sentry/CPEMan_web/Makefile
TianShan/Sentry/ClibMan_web/Makefile
TianShan/Sentry/Module/Storage_web/Makefile
TianShan/Sentry/service/Makefile
TianShan/StreamSmith/Makefile
TianShan/StreamSmith/StreamClient/Makefile
TianShan/StreamSmith/Modules/SsmTianShanS1_Now/Makefile
TianShan/StreamSmith/LSCP/lscpLib/Makefile
TianShan/StreamSmith/Service/Makefile
TianShan/StreamService/Makefile
TianShan/StreamService/ServiceLib/Makefile
TianShan/EventGateway/Makefile
TianShan/EventGateway/Modules/EGH_SnmpTrap/Makefile
TianShan/EventGateway/Modules/EGH_JMS/Makefile
TianShan/EventGateway/Modules/EGH_FileWriter/Makefile
TianShan/EventGateway/service/Makefile
TianShan/Action/A3Message/Makefile
TianShan/EventRE/Makefile
TianShan/ContentLib/ContentLibClient/Makefile
TianShan/ContentLib/Makefile
Generic/TsHammer/Makefile
TianShan/AccreditedPath/Makefile
TianShan/AccreditedPath/pho/pho_SeaChange/Makefile
TianShan/VirtualStreamingService/Makefile
TianShan/VirtualStreamingService/Pho/Makefile
Common/RtspLib/RtspClientLib/Makefile
TianShan/EdgeRM/Makefile
TianShan/EdgeRM/Pho_ERM/Makefile
TianShan/Weiwoo/Makefile
TianShan/Weiwoo/service/Makefile
TianShan/AccreditedPath/admin/Makefile
TianShan/SiteAdmin/Makefile
TianShan/application/Makefile
TianShan/application/MODPlugIn/Makefile
TianShan/application/MOD2/Makefile
TianShan/DummySS/Makefile
TianShan/StreamService/StreamPumper/Makefile
TianShan/ContentStore/A3CS/Makefile
TianShan/StreamService/NSS/Makefile
Generic/xmlshd/Makefile
test/libtool_use/Makefile
])
