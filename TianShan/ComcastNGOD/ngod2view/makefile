TOPDIR := ../../../

include $(TOPDIR)/build/defines.mk

SOLIB := ngod2view

_files := ngod.ice

OBJS :=  BasePage.os MainPage.os  NgodPage.os PluginEntry.os  ngod.os

INCDIR += $(TOPDIR)/Common $(TOPDIR)/TianShan/common $(TOPDIR)/TianShan/Ice $(TOPDIR)/TianShan/Sentry

LDFLAGS += -lZQCommon -lTianShanIce -lTianShanCommon $(icelink)

include $(TOPDIR)/build/common.mk

%.h %.cpp: %.ice
	@echo "compiling ICE file $< ...."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) -I$(_ice_slice_dir)/Ice $<
