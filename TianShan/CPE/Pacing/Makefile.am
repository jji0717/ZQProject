include  $(top_srcdir)/config/compileflags.mk
CXXFLAGS+=-I.. -I../common -I@_vstrm_inc_dir@  -I@_ctf_inc_dir@
CXXFLAGS+=-DPACING_EXPORTS -DLOGFMTWITHTID -D_LINUX
 
VPATH=..:../common:@VPATHS@:
SOURCES:=Pacing.cpp PacedIndex.cpp PacingLogic.cpp VvcIndexHelper.cpp

AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libPacing.la
libPacing_la_SOURCES=$(SOURCES)
libPacing_la_LIBADD=
#libPacing_la_LDFLAGS=@lib_versioninfo@
AM_LDFLAGS=$(commonlink) $(icelink) -L@_ctf_lib_dir@ $(LDFLAGS)

.SECONDARY: $(GENFILES)
.PHONY:all clean test gent icetest
clean-local:
	-rm -rf $(csbin_dir)/$(lib_LTLIBRARIES:%.la=%)*  $(cssyb_dir)/$(lib_LTLIBRARIES:%.la=%)*
tsflag:="so"
goal:=libPacing.so
include $(top_srcdir)/config/endpros.mk


