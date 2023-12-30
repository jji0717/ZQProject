TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

SOLIB := MHO_GBMOD

_ice_files = LAMFacade.ice LSMSForMoD.ice ote.ice Surf_Tianshan.ice ADM.ice
_dir1 := ../ICE/
_files := $(addprefix $(_dir1), $(_ice_files))

GENFILES := $(patsubst %.ice, %.h, $(_ice_files)) $(patsubst %.ice, %.cpp, $(_ice_files))

OBJS := $(patsubst %.ice, %.os, $(_ice_files)) \
	LAMPlayListQuery.os LAMPlayListQuery3.os LSMSForMoDAuthorization.os LSMSForMoDForTeardownCB.os \
	ADMPlacement.os OTEAuthorization.os OTEForTeardownCB.os MODPlugIn.os AAAQuery.os HeNanAAAQuery.os \
	ConfigHelper.os

LDFLAGS += $(commonlink)

INCDIR += ../GB_MOD2

include $(TOPDIR)/build/common.mk

LAMFacade.h LAMFacade.cpp: $(word 1, $(_files))
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $< 

LSMSForMoD.h LSMSForMoD.cpp: $(word 2, $(_files))
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $< 

ote.h ote.cpp: $(word 3, $(_files))
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $< 

Surf_Tianshan.h Surf_Tianshan.cpp: $(word 4, $(_files))
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $< 

ADM.h ADM.cpp: $(word 5, $(_files))
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $< 
