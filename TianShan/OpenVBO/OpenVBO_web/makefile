TOPDIR := ../../../

include $(TOPDIR)/build/defines.mk

SOLIB := OpenVBO_web
SsmOpenVBO_files := $(TOPDIR)/TianShan/OpenVBO/OpenVBO_web/SsmOpenVBO.ICE

OBJS := OpenVBO_web.os OpenVBOClient.os SsmOpenVBO.os
				
INCDIR += $(TOPDIR)/TianShan/Sentry $(TOPDIR)/TianShan/common $(TOPDIR)/TianShan/Ice $(TOPDIR)/Common $(ICE_ROOT)/include $(ICE_ROOT)/include/stlport

VPATH += .:..:../../common:$(TOPDIR)/Common:$(TOPDIR)/TianShan/Ice

LDFLAGS += $(icelink) $(commonlink)

include $(TOPDIR)/build/common.mk

SsmOpenVBO.h SsmOpenVBO.cpp: $(SsmOpenVBO_files)
	@echo "compiling ICE file $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $< --output-dir .
