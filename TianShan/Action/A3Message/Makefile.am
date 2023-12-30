include  $(top_srcdir)/config/compileflags.mk
CXXFLAGS+=-I../../EventRE 
CXXFLAGS+=-DWITH_ICESTORM
VPATH=:@VPATHS@:

AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libAction_A3Message.la
libAction_A3Message_la_SOURCES=HttpClient.cpp A3Call.cpp A3HttpReq.cpp A3Message.cpp 
libAction_A3Message_la_LIBADD=
#libAction_A3Message_la_LDFLAGS=@lib_versioninfo@
AM_LDFLAGS=$(commonlink) $(icelink)

.PHONY:rubbish
HttpClient.lo : rubbish
rubbish : HttpClient.o
	touch rubbish
clean-local:
	-rm -rf $(csbin_dir)/$(lib_LTLIBRARIES:%.la=%)*  $(cssyb_dir)/$(lib_LTLIBRARIES:%.la=%)*
tsflag:="so"
goal:=libAction_A3Message.so
include  $(top_srcdir)/config/endpros.mk
