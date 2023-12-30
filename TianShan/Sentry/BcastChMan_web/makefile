TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

SOLIB := BcastChMan_web

ice_file := $(TOPDIR)/TianShan/BroadcastChannel/BcastChannelEx.ICE
general := BcastChannelEx.h BcastChannelEx.cpp

OBJS := AddChannel.os AddFilterItem.os BasePage.os BcastChannelEx.os BcastChMan_web.os BcastMain.os EditChannel.os EditItem.os InsertItem.os PushItem.os RemoveChannel.os RemoveFilterItem.os RemoveItem.os ShowChannel.os ShowItem.os stroprt.os

INCDIR += $(TOPDIR)/common $(TOPDIR)/TianShan/Ice $(TOPDIR)/TianShan/BroadcastChannel $(ICE_ROOT)/include

VPATH += $(TOPDIR)/TianShan/BroadcastChannel

LDFlAGS += -lZQCommon -lTianShanCommon -lTianShanIce -lSNMPManPkg -lSNMPAgent

include $(TOPDIR)/build/common.mk

$(general):$(ice_file)
	@echo "compiling ICE file $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $< --output-dir .
