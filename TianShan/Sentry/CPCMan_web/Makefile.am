include  $(top_srcdir)/config/compileflags.mk

AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libCPCMan_web.la
libCPCMan_web_la_SOURCES=BasePage.cpp CPCMan_web.cpp CpcMethodController.cpp CpcServiceController.cpp stroprt.cpp
libCPCMan_web_la_LIBADD=
#libCPCMan_web_la_LDFLAGS=@lib_versioninfo@
AM_LDFLAGS=
clean-local:
	-rm -rf $(csbin_dir)/$(lib_LTLIBRARIES:%.la=%)*  $(cssyb_dir)/$(lib_LTLIBRARIES:%.la=%)*
tsflag:="so"
goal:=libCPCMan_web.so
include $(top_srcdir)/config/endpros.mk

