include  $(top_srcdir)/config/compileflags.mk

CXXFLAGS+=-DWITH_ICESTORM -I../../../Common/HttpEngine
VPATH=@VPATHS@
AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libCRM_A3Message.la
libCRM_A3Message_la_SOURCES=A3Client.cpp A3MessageEventSinkI.cpp A3MessageHandler.cpp A3MsgCommon.cpp \
							 A3MsgEnv.cpp CRM_A3Message.cpp ConfigHelper.cpp HttpClient.cpp
libCRM_A3Message_la_LIBADD=	 
#libCRM_A3Message_la_LDFLAGS=@lib_versioninfo@
#AM_CPPFLAGS=${CXXFLAGS}
AM_LDFLAGS=$(commonlink) $(icelink) $(LDFLAGS) 
.PHONY: rubbish
ConfigHelper.lo  HttpClient.lo :rubbish
rubbish:HttpClient.o ConfigHelper.o
	size $^
	touch rubbish
clean-local:
	-rm -rf $(csbin_dir)/$(lib_LTLIBRARIES:%.la=%)*  $(cssyb_dir)/$(lib_LTLIBRARIES:%.la=%)*
tsflag:="so"
goal:=libCRM_A3Message.so
include  $(top_srcdir)/config/endpros.mk
