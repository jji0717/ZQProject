include  $(top_srcdir)/config/compileflags.mk


AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libTSClient.la
libTSClient_la_SOURCES=SNMPOper.cpp VarCache.cpp TSClient.cpp 
libTSClient_la_LIBADD=
#libTSClient_la_LDFLAGS=@lib_versioninfo@

AM_LDFLAGS=$(commonlink) -lsnmp $(LDFLAGS)
clean-local:
	-rm -rf $(csbin_dir)/$(lib_LTLIBRARIES:%.la=%)*  $(cssyb_dir)/$(lib_LTLIBRARIES:%.la=%)*
tsflag:="so"
goal:=libTSClient.so
include  $(top_srcdir)/config/endpros.mk
