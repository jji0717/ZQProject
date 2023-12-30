include  $(top_srcdir)/config/compileflags.mk 
VPATH=:@VPATHS@:
AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libPHO_VSS.la
libPHO_VSS_la_SOURCES=ConfigHelper.cpp Configuration.cpp pho_VSS.cpp PhoCisco.cpp	\
			PhoHsnTree.cpp PhoNGOD.cpp PhoNGOD_DVBC.cpp phoStorageLink.cpp public.cpp 

libPHO_VSS_la_LIBADD=
#libPHO_VSS_la_LDFLAGS=@lib_versioninfo@
AM_LDFLAGS=$(commonlink) $(icelink)
clean-local:
	-rm -rf $(csbin_dir)/$(lib_LTLIBRARIES:%.la=%)*  $(cssyb_dir)/$(lib_LTLIBRARIES:%.la=%)*
tsflag:="so"
goal:=libPHO_VSS.so
include  $(top_srcdir)/config/endpros.mk
