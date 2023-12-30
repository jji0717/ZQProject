include  $(top_srcdir)/config/compileflags.mk

CXXFLAGS+=-I../../EventSink

VPATH=:@VPATHS@:
AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libSnmpPlug.la
libSnmpPlug_la_SOURCES=ConfigHelper.cpp snmpplug.cpp SnmpSender.cpp
libSnmpPlug_la_LIBADD=
#libSnmpPlug_la_LDFLAGS=@lib_versioninfo@
AM_LDFLAGS=-lZQCommon -lSnmpManPkg -lsnmp++
clean-local:
	-rm -rf $(csbin_dir)/$(lib_LTLIBRARIES:%.la=%)*  $(cssyb_dir)/$(lib_LTLIBRARIES:%.la=%)*
tsflag:="so"
goal:=libSnmpPlug.so
include $(top_srcdir)/config/endpros.mk
