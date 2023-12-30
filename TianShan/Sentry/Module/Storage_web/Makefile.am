include  $(top_srcdir)/config/compileflags.mk

CXXFLAGS+=-I../..

AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libStorage_web.la
libStorage_web_la_SOURCES=Controller.cpp Storage_web.cpp StringUtil.cpp
libStorage_web_la_LIBADD=
#libStorage_web_la_LDFLAGS=@lib_versioninfo@
AM_LDFLAGS=
clean-local:
	-rm -rf $(csbin_dir)/$(lib_LTLIBRARIES:%.la=%)*  $(cssyb_dir)/$(lib_LTLIBRARIES:%.la=%)*
tsflag:="so"
goal:=libStorage_web.so
include $(top_srcdir)/config/endpros.mk

