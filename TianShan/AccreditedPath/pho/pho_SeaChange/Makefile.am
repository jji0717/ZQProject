include  $(top_srcdir)/config/compileflags.mk
CXXFLAGS+=-I..

SOURCESALL:=ConfigHelper.cpp  ../Config.cpp ../IpEdgePHO.cpp ../public.cpp 	\
		../Raid5sqrPHO.cpp ../pho_SeaChange.cpp

AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libPHO_SeaChange.la
libPHO_SeaChange_la_SOURCES=
libPHO_SeaChange_la_LIBADD=
#libPHO_SeaChange_la_LDFLAGS=@lib_versioninfo@
AM_LDFLAGS=$(commonlink) $(icelink) $(LDFLAGS)
clean-local:
	-rm -rf $(csbin_dir)/$(lib_LTLIBRARIES:%.la=%)*  $(cssyb_dir)/$(lib_LTLIBRARIES:%.la=%)*
tsflag:="so"
goal:=libPHO_SeaChange.so
include $(top_srcdir)/config/endpros.mk
