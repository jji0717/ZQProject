include $(top_srcdir)/config/compileflags.mk

CXXFLAGS+=-DNEWLOGFMT
AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libWebLayout.la
libWebLayout_la_SOURCES=ConsoleCommand.cpp DataSourceLoader.cpp GridRequest.cpp LayoutCtx.cpp LogRequest.cpp \
			SnmpRequest.cpp WebLayout.cpp
libWebLayout_la_LIBADD=
#libWebLayout_la_LDFLAGS=@lib_versioninfo@
AM_LDFLAGS=-lZQCommon
clean-local:
	-rm -rf $(csbin_dir)/$(lib_LTLIBRARIES:%.la=%)*  $(cssyb_dir)/$(lib_LTLIBRARIES:%.la=%)*
tsflag:="so"
goal:=libWebLayout.so
include  $(top_srcdir)/config/endpros.mk
