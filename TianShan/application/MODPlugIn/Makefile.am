include  $(top_srcdir)/config/compileflags.mk

_ice_files = LAMFacade.ice LSMSForMoD.ice ote.ice Surf_Tianshan.ice ADM.ice
_dir1 := ../ICE/
_files := $(addprefix $(_dir1), $(_ice_files))
GENFILES := $(patsubst %.ice, %.h, $(_ice_files)) $(patsubst %.ice, %.cpp, $(_ice_files))
SOURCESALL:=LAMFacade.cpp LSMSForMoD.cpp ote.cpp Surf_Tianshan.cpp  \
		ADM.cpp LAMPlayListQuery.cpp LAMPlayListQuery3.cpp LSMSForMoDAuthorization.cpp 	\
		LSMSForMoDForTeardownCB.cpp ADMPlacement.cpp OTEAuthorization.cpp 	\
		OTEForTeardownCB.cpp MODPlugIn.cpp LAMProviderQuery.cpp

AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libMHO_MOD.la
libMHO_MOD_la_SOURCES=$(SOURCESALL)
libMHO_MOD_la_LIBADD=
#libMHO_MOD_la_LDFLAGS=@lib_versioninfo@
AM_LDFLAGS=$(commonlink)
LAMFacade.h LAMFacade.cpp: $(word 1, $(_files))
	$(SLICE2CPP) $(SLICEFLAGS) $<
LSMSForMoD.h LSMSForMoD.cpp: $(word 2, $(_files))
	$(SLICE2CPP) $(SLICEFLAGS) $<
ote.h ote.cpp: $(word 3, $(_files))
	$(SLICE2CPP) $(SLICEFLAGS) $<
Surf_Tianshan.h Surf_Tianshan.cpp: $(word 4, $(_files))
	$(SLICE2CPP) $(SLICEFLAGS) $<
ADM.h ADM.cpp: $(word 5, $(_files))
	$(SLICE2CPP) $(SLICEFLAGS) $<
clean-local:
	-rm -rf {ADM,LAMFacade,LSMSForMoD,ote,Surf_Tianshan}.{h,cpp} $(csbin_dir)/$(lib_LTLIBRARIES:%.la=%)*  $(cssyb_dir)/$(lib_LTLIBRARIES:%.la=%)*
tsflag:="so"
goal:=libMHO_MOD.so
include $(top_srcdir)/config/endpros.mk
