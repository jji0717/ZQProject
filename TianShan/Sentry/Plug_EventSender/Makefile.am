include  $(top_srcdir)/config/compileflags.mk
CXXFLAGS+=-I../EventSink -I$(top_srcdir)/Generic/JndiClient
CXXFLAGS+=-D_ICE_INTERFACE_SUPPORT

VPATH=:@VPATHS@:
AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libEventSender.la
libEventSender_la_SOURCES=ConfigHelper.cpp BaseSender.cpp TextWriter.cpp EventSender.cpp \
							IceSender.cpp JmsSender.cpp MagSender.cpp StdAfx.cpp
libEventSender_la_LIBADD=
#libEventSender_la_LDFLAGS=@lib_versioninfo@
AM_LDFLAGS=$(commonlink) $(icelink) $(snmplink) -lJndiClient
clean-local:
	-rm -rf $(csbin_dir)/$(lib_LTLIBRARIES:%.la=%)*  $(cssyb_dir)/$(lib_LTLIBRARIES:%.la=%)*
tsflag:="so"
goal:=libEventSender.so
include  $(top_srcdir)/config/endpros.mk
