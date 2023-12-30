TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

TARGET := GBMODApp

_ice_files = LAMFacade.ice ModSvcIce.ice ote.ice Surf_Tianshan.ice 
_dir1 := ../ICE/
_files := $(addprefix $(_dir1), $(_ice_files))

GENFILES := $(patsubst %.ice, %.h, $(_ice_files)) $(patsubst %.ice, %.cpp, $(_ice_files)) \
	    Stream2Purchase.h Stream2Purchase.cpp ModSvcIce.h ModSvcIce.cpp

OBJS := $(patsubst %.ice, %.o, $(_ice_files)) Stream2Purchase.o \
	ZQDaemon.o ConfigHelper.o MODHelperMgr.o ModService.o ModSvcIceImpl.o WatchDog.o 

LDFLAGS += -lServiceShell $(commonlink) $(icelink) $(snmplink) -lboost_regex -lpthread -ldl

include $(TOPDIR)/build/common.mk

SLICEFLAGS += -I$(_dir1)

LAMFacade.h LAMFacade.cpp: $(word 1, $(_files))
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $< 

ModSvcIce.h ModSvcIce.cpp Stream2Purchase.h Stream2Purchase.cpp: $(word 2, $(_files)) Surf_Tianshan.h 
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $<
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
	--index "ZQTianShan::Application::MOD::Stream2Purchase,ZQTianShan::Application::MOD::ModPurchase,streamId" Stream2Purchase $<

ote.h ote.cpp: $(word 3, $(_files))
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $< 

Surf_Tianshan.h Surf_Tianshan.cpp: $(word 4, $(_files))
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $< 
