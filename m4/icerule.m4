dnl obtain env-value from configure.ac
_prefix=$PROJSPATH
_sdk_dir=$THIRDSDKPATH

dnl export some variables via macro AC_SUBST,so that anywhere Makefile.am use it.
_schange_sdk=${_sdk_dir}/SeaChangeKits
_3rdparty_sdk=${_sdk_dir}/3rdPartyKits

AC_MSG_NOTICE([romalin11 _prefix=${_prefix} ....._sdk_dir=${_sdk_dir}]) 
_tsdir=${_prefix}/TianShan
_csdir=${_tsdir}/ContentStore

_sys_libdir=/usr/lib64/
_bindir=${_tsdir}/bin64
_libdir=${_tsdir}/lib64
_sym_dir=${_prefix}/symbols

dnl third party SDK
_java_dir=${_3rdparty_sdk}/java
_java_hdr_diro=${_java_dir}/include 
_java_hdr_dirt=${_java_dir}/include/linux
_ice_dir=${_3rdparty_sdk}/ice
_ice_lib_dir=${_ice_dir}/lib
_ice_slice_dir=${_ice_dir}/slice

_net_snmp_dir=${_3rdparty_sdk}/net-snmp
_snmppp_dir=${_3rdparty_sdk}/snmp++
_expat_dir=${_3rdparty_sdk}/expat
_libnl_dir=${_3rdparty_sdk}/libnl
_pcap_dir=${_3rdparty_sdk}/libpcap
_libaio_dir=${_3rdparty_sdk}/libaio
_boost_dir=${_3rdparty_sdk}/boost
_e2fsprogs_dir=${_3rdparty_sdk}/e2fsprogs
_readline_dir=${_3rdparty_sdk}/readline
_openssl_dir=${_3rdparty_sdk}/openssl
_krb5_dir=${_3rdparty_sdk}/krb5
_termcap_dir=${_3rdparty_sdk}/libtermcap
_ncurses_dir=${_3rdparty_sdk}/ncurses

_vstrm_dir=${_schange_sdk}/vstrmkit
_ctf_dir=${_schange_sdk}/ctflib
_ctf_inc_dir=${_ctf_dir}/Include
_ctf_lib_dir=${_ctf_dir}/LINUX64_REL
_vstrm_inc_dir=${_vstrm_dir}/Sdk/inc

INCDIRS="${_prefix}/Common ${_prefix}/build ${_tsdir}/common ${_tsdir}/Ice ${_tsdir}/Shell/ZQSNMPManPkg ${_csdir} ${_ice_dir}/include ${_ice_slice_dir} ${_net_snmp_dir}/include ${_snmppp_dir}/include ${_expat_dir}/include ${_pcap_dir}/include ${_libaio_dir}/include ${_libnl_dir}/include ${_boost_dir}/include ${_e2fsprogs_dir}/include ${_java_hdr_diro} ${_java_hdr_dirt} ${_readline_dir}/include ${_openssl_dir}/include ${_krb5_dir}/include ${_termcap_dir}/include ${_ncurses_dir}/include"
LINKDIRS="${_bindir} ${_libdir} ${_ice_dir}/lib ${_net_snmp_dir}/lib ${_snmppp_dir}/lib ${_expat_dir}/lib ${_pcap_dir}/lib ${_libaio_dir}/lib  ${_libnl_dir}/lib ${_boost_dir}/lib ${_e2fsprogs_dir}/lib ${_readline_dir}/lib ${_openssl_dir}/lib ${_krb5_dir}/lib ${_termcap_dir}/lib ${_ncurses_dir}/lib"
AC_MSG_NOTICE([romalin22 INCDIRS=${INCDIRS} .....LINKDIRS=${LINKDIRS}]) 

dnl _sys_interp=/lib64/ld-linux-x86-64.so.2

VPATHS+=${_prefix}/Common:${_tsdir}/Ice:${_csdir}:${_csdir}/ICE:${_tsdir}/Shell/ZQSNMPManPkg/
AC_MSG_NOTICE([romalin33  VPATHS=${VPATHS}]) 

dnl export varaiable or setting output variables
dnl Macro AC_SUBST(variable ,[value])
AC_SUBST(INCDIRS)
AC_SUBST(LINKDIRS)

AC_SUBST(_bindir)
AC_SUBST(_libdir)
AC_SUBST(_sym_dir)

AC_SUBST(_java_dir)
AC_SUBST(_tsdir)
AC_SUBST(_ice_dir)
AC_SUBST(_ice_lib_dir)
AC_SUBST(_ice_slice_dir)
dnl AC_SUBST( _sys_interp)

AC_SUBST(_net_snmp_dir)
AC_SUBST(_snmppp_dir)
AC_SUBST(_expat_dir)
AC_SUBST(_libnl_dir)
AC_SUBST(_pcap_dir)
AC_SUBST(_libaio_dir)
AC_SUBST(_boost_dir)
AC_SUBST(_e2fsprogs_dir)
AC_SUBST(_readline_dir)
AC_SUBST(_openssl_dir)
AC_SUBST(_krb5_dir)
AC_SUBST(_termcap_dir)
AC_SUBST(_ncurses_dir)


AC_SUBST(VPATHS)

AC_SUBST(_vstrm_inc_dir)
AC_SUBST(_ctf_inc_dir)
AC_SUBST(_ctf_lib_dir)
dnl # Configurable Ice locations

AC_MSG_NOTICE([**** qazwsx ${_prefix}]) 

AC_MSG_NOTICE([**** Checking 3rdPartyKits support:])

dnl 3rd packages
AC_ARG_WITH(3rd,
            AC_HELP_STRING(
                [--with-3rd=PREFIX],
                [location of the 3rdPartyKits PREFIX.(ex. /opt/sdk)]),
            [CPPFLAGS="-I$withval/3rdPartyKits/ice/include $CPPFLAGS"
              LDFLAGS="-L$withval/3rdPartyKits/ice/lib  $LDFLAGS"
              ICEPATH=$withval/3rdPartyKits/ice
              if test -e "$withval"; then
                  AC_SUBST(ICEPATH)
                  AC_MSG_NOTICE(using 3rdPartyKits directory... $withval)
              elif test -e $withval/share/ice/slice; then 
                  AC_SUBST(3rdPartyKits_DIR,"$withval/share/ice/slice")
                  AC_MSG_NOTICE(using Ice 3rdPartyKits_DIR directory... $withval/share/ice/slice)
              else
                  AC_MSG_ERROR("Cannot find slice directory:$withval/slice or $withval/share/ice/slice")
              fi
             AC_MSG_NOTICE(using 3rdPartyKits include directory... $withval/*/include)
             AC_MSG_NOTICE(using 3rdPartyKits library directory... $withval/*/lib)],
            [if test -e /usr/share/Ice/slice; then
                AC_SUBST(SLICE_DIR,"/usr/share/Ice/slice")
             else
                AC_MSG_ERROR("Cannot find slice directory: /usr/share/Ice/slice")
             fi])

dnl # Ice checks and variables definitions
dnl # If package found HAVE_ICE is defined in config header and with_ice is /= no
dnl AC_CHECK_HEADER(header-file,[action-if-found],[action-if-not-found])
AC_LANG_PUSH([C++])
ERRORS=""
AC_CHECK_HEADERS([Ice/Ice.h IceUtil/IceUtil.h IceBox/IceBox.h IceStorm/IceStorm.h],
    [],
    [ERRORS="$ac_header not found"]) 
AC_CHECK_LIB([Ice],[main],
    [
dnl	AC_SUBST([LIBICE],["-lIce"])
	AC_DEFINE([HAVE_LIBICE],[1],[Define if you have libIce])
    ],
    [ERRORS="$ERRORS, libIce not found"])
AC_CHECK_LIB([IceUtil],[main],
    [
dnl	AC_SUBST([LIBICEUTIL],["-lIceUtil"])
	AC_DEFINE([HAVE_LIBICEUTIL],[1],[Define if you have libIceUtil])
    ],
    [ERRORS="$ERRORS, libIceUtil not found"])
AC_CHECK_LIB([IceGrid],[main],
    [
dnl	AC_SUBST([LIBICEGRID],["-lIceGrid"])
	AC_DEFINE([HAVE_LIBICEGRID],[1],[Define if you have libIceGrid])
    ],
    [ERRORS="$ERRORS, libIceGrid not found"])
AC_CHECK_LIB([IceBox],[main],
    [
dnl	AC_SUBST([LIBICEBOX],["-lIceBox"])
	AC_DEFINE([HAVE_LIBICEBOX],[1],[Define if you have libIceBox])
    ],
    [ERRORS="$ERRORS, libIceBox not found"])
AC_CHECK_LIB([IceStorm],[main],
    [
dnl	AC_SUBST([LIBICESTORM],["-lIceStorm"])
	AC_DEFINE([HAVE_LIBICESTORM],[1],[Define if you have libIceStorm])
    ],
    [ERRORS="$ERRORS, libIceStorm not found"])
AC_CHECK_LIB([IceStormService],[main],
    [
dnl	AC_SUBST([LIBICESTORMSERVICE],["-lIceStormService"])
	AC_DEFINE([HAVE_LIBICESTORMSERVICE],[1],[Define if you have libIceStormService])
    ],
    [ERRORS="$ERRORS, libIceStormService not found"])
AC_LANG_POP([C++])

AC_ARG_VAR([SLICE2CPP],[command use to generate c++ code from slice])
AC_ARG_VAR([SLICE2JAVA],[command use to generate java code from slice])
AC_ARG_VAR([SLICE2PYTHON],[command use to generate python code from slice])


dnl AC_PATH_PROG([SLICE2CPP],[slice2cpp],[no])
AC_PATH_PROG([SLICE2CPP],[slice2cpp],$ICEPATH/bin/slice2cpp,$PATH:/opt/3rdpks/ice)
if test "x$SLICE2CPP" = xno; then
    ERRORS="$ERRORS, could not find slice2cpp needed to build c++ interfaces code"
fi


dnl AC_PATH_PROG([SLICE2JAVA],[slice2java],[no])
AC_PATH_PROG([SLICE2JAVA],[slice2java],$ICEPATH/ice/bin/slice2java,$PATH:/opt/3rdpks/ice)
if test "x$SLICE2JAVA" = xno; then
    ERRORS="$ERRORS, could not find slice2java needed to build java interfaces code"
fi

dnl AC_PATH_PROG([SLICE2PYTHON],[slice2py],[no])
AC_PATH_PROG([SLICE2PYTHON],[slice2py],$ICEPATH/ice/bin/slice2py,$PATH:/opt/3rdpks/ice)
if test "x$SLICE2PYTHON" = xno; then
    ERRORS="$ERRORS, could not find slice2py needed to build python interfaces code"
fi

dnl #define SLICEDIR if empty
if test -z "$SLICEDIR"; then
   SLICEDIR="/usr/share/slice"
fi

if test "$ERRORS"; then
    AC_MSG_NOTICE([Errors found checking Ice support: $ERRORS. Ice support disabled])
    with_ice="no"
else
    AC_DEFINE([HAVE_ICE],[1],[Defined if Ice found])
dnl    AC_SUBST(SLICE2CPP)
dnl    AC_SUBST([ICE_CPPFLAGS])
    AC_SUBST([ICE_LDFLAGS],["$LIBICE $LIBICEUTIL $LIBICEGRID $LIBICEBOX $LIBICESTORM $LIBICESTORMSERVICE"])
fi

