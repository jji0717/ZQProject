include  $(top_srcdir)/config/compileflags.mk

AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libSnmpManPkg.la
libSnmpManPkg_la_SOURCES=SnmpManPkg.cpp  SnmpSubagent.cpp SnmpUtil.cpp VarInfo.cpp Mib.cpp \
			Var.cpp 
#libSnmpManPkg_la_LDFLAGS=@lib_versioninfo@
AM_LDFLAGS=  -lsnmp $(LDFlAGS)

#all:
#	install -D  $(lib_LTLIBRARIES)  $(top_srcdir)/TianShan/bin64
#	cp  -R  .libs/libSnmpManPkg.so*   $(top_srcdir)/TianShan/bin64/.libs/
clean-local:
	-rm -rf $(csbin_dir)/$(lib_LTLIBRARIES:%.la=%)*  $(cssyb_dir)/$(lib_LTLIBRARIES:%.la=%)*
tsflag:="so"
goal:=libSnmpManPkg.so
include  $(top_srcdir)/config/endpros.mk

