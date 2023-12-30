TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

_files := $(TOPDIR)/TianShan/EdgeRM/EdgeRM.ICE
GENFILES := $(TOPDIR)/TianShan/EdgeRM/EdgeRM.cpp $(TOPDIR)/TianShan/EdgeRM/EdgeRM.h

SOLIB := ErmMan_web

OBJS := EdgeRM.os AddChannel.os AddDevice.os BasePage.os ChannelDetail.os DataTypes.os EditChannel.os EditDevice.os \
				EditQAM.os EditRouteNames.os EditServiceGroup.os ErmMain.os ErmMan_web.os RemoveChannel.os RemoveDevice.os \
				ShowAllocation.os ShowChannel.os ShowDevice.os ShowEdgePort.os ShowRouteNames.os ShowServiceGroup.os stroprt.os
				
INCDIR += $(TOPDIR)/TianShan/EdgeRM $(TOPDIR)/TianShan/ice $(TOPDIR)/TianShan/common $(TOPDIR)/Common $(ICE_ROOT)/include $(ICE_ROOT)/include/stlport

VPATH += .:..:../../common:$(TOPDIR)/Common:$(TOPDIR)/TianShan/EdgeRM:$(TOPDIR)/TianShan/ice

LDFLAGS += $(icelink) $(commonlink)

include $(TOPDIR)/build/common.mk

$(TOPDIR)/TianShan/EdgeRM/EdgeRM.h $(TOPDIR)/TianShan/EdgeRM/EdgeRM.cpp: $(_files)
	@echo "compiling ICE file $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $< --output-dir $(dir $(_files))