include  $(top_srcdir)/config/compileflags.mk

AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libCPEMan_web.la
libCPEMan_web_la_SOURCES=BasePage.cpp CpeMan_web.cpp MethodController.cpp SessionController.cpp stroprt.cpp
libCPEMan_web_la_LIBADD=
#libCPEMan_web_la_LDFLAGS=@lib_versioninfo@
AM_LDFLAGS=$(commonlink) $(icelink)
clean-local:
	-rm -rf $(csbin_dir)/$(lib_LTLIBRARIES:%.la=%)*  $(cssyb_dir)/$(lib_LTLIBRARIES:%.la=%)*
tsflag:="so"
goal:=libCPEMan_web.so
include  $(top_srcdir)/config/endpros.mk
