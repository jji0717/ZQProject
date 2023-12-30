TOPDIR := ../../../..

include $(TOPDIR)/build/defines.mk

SOLIB := c2loc_web

OBJS := c2loc_web.os C2Locator.os

INCDIR += $(TOPDIR)/TianShan/Sentry ../..

_file := ../../CRM_C2Locator/C2Locator.ice
GENFILES := C2Locator.h C2Locator.cpp

LDFLAGS := -lZQCommon -lTianShanCommon -lTianShanIce $(icelink) 

include $(TOPDIR)/build/common.mk

$(GENFILES): $(_file)
	@printf "%-18s %-4s %s\n" "[$(SOLIB)]" "ICE" $<
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $<
    




