include $(top_srcdir)/config/compileflags.mk
CXXFLAGS+=-I$(top_srcdir)/TianShan/common/snmp
VPATH:=:@VPATHS@:
AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libSnmpAgent.la
libSnmpAgent_la_SOURCES=SnmpUtil.cpp SnmpAgent.cpp
libSnmpAgent_la_LIBADD=
libSnmpAgent_la_LDFLAGS=
AM_LDFLAGS=-lZQCommon -lsnmp
.PHONY:rubbish
SnmpUtil.lo : rubbish 
rubbish : SnmpUtil.o
	touch rubbish
clean-local:
	-rm -rf $(csbin_dir)/$(lib_LTLIBRARIES:%.la=%)*  $(cssyb_dir)/$(lib_LTLIBRARIES:%.la=%)*
tsflag:="so"
goal:=libSnmpAgent.so
include $(top_srcdir)/config/endpros.mk
