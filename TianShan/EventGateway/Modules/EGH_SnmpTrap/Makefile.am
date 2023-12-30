include  $(top_srcdir)/config/compileflags.mk

CXXFLAGS+=-I../.. -I../../../Sentry/EventSink
VPATH=:@VPATHS@:

AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libEGH_SnmpTrap.la
libEGH_SnmpTrap_la_SOURCES=SnmpSender.cpp EGH_SnmpTrap.cpp ConfigHelper.cpp
libEGH_SnmpTrap_la_LIBADD=
#libEGH_SnmpTrap_la_LDFLAGS=@lib_versioninfo@
AM_LDFLAGS=-lsnmp++
clean-local:
	-rm -rf $(csbin_dir)/$(lib_LTLIBRARIES:%.la=%)*  $(cssyb_dir)/$(lib_LTLIBRARIES:%.la=%)*
tsflag:="so"
goal:=libEGH_SnmpTrap.so
include $(top_srcdir)/config/endpros.mk
	

