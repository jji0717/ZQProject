include  $(top_srcdir)/config/compileflags.mk
CXXFLAGS+=-I../../EventSink
VPATH=@VPATHS@

AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libMsgSender.la
libMsgSender_la_SOURCES=BaseSender.cpp ConfigHelper.cpp IceSender.cpp MagSender.cpp TextWriter.cpp
libMsgSender_la_LIBADD=
#libMsgSender_la_LDFLAGS=@lib_versioninfo@
AM_LDFLAGS=$(commonlink) $(icelink)
clean-local:
	-rm -rf $(csbin_dir)/$(lib_LTLIBRARIES:%.la=%)*  $(cssyb_dir)/$(lib_LTLIBRARIES:%.la=%)*
tsflag:="so"
goal:=libMsgSender.so
include $(top_srcdir)/config/endpros.mk


