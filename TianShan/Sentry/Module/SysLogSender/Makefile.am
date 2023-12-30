include  $(top_srcdir)/config/compileflags.mk

CXXFLAGS+=-I../../EventSink
CXXFLAGS+=-DCONFIG_NO_SNMP
VPATH=:@VPATHS@:

AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libSysLogger.la
libSysLogger_la_SOURCES=ConfigHelper.cpp RemoteSyslog.cpp SysLogger.cpp
libSysLogger_la_LIBADD=
#libSysLogger_la_LDFLAGS=@lib_versioninfo@
AM_LDFLAGS=-lZQCommon
clean-local:
	-rm -rf $(csbin_dir)/$(lib_LTLIBRARIES:%.la=%)*  $(cssyb_dir)/$(lib_LTLIBRARIES:%.la=%)*
tsflag:="so"
goal:=libSysLogger.so
include $(top_srcdir)/config/endpros.mk


