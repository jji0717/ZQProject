TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

TARGET := MODSvc

_ice_files = LAMFacade.ice ModSvcIce.ice ote.ice Surf_Tianshan.ice
_dir1 := ../ICE/
_dir2  := ../../DummySS
_files := $(addprefix $(_dir1), $(_ice_files))
_files2 := $(_dir2)/StreamSmithAdmin.ICE

GENFILES := $(patsubst %.ice, %.h, $(_ice_files)) $(patsubst %.ice, %.cpp, $(_ice_files)) \
	    Stream2Purchase.h Stream2Purchase.cpp ModSvcIce.h ModSvcIce.cpp \
		StreamSmithAdmin.h StreamSmithAdmin.cpp

OBJS := $(patsubst %.ice, %.o, $(_ice_files)) ServiceGroupPump.o Stream2Purchase.o StreamSmithAdmin.o \
	ZQDaemon.o ConfigHelper.o MODHelperMgr.o ModService.o ModSvcIceImpl.o WatchDog.o \
	MRTClient.o MRTStreamService.o soapC.o stdsoap2.o soapClient.o MODMRTProxy.o
	
INCDIR += . ../.. ../../BroadcastChannel/soap ../../BroadcastChannel ../../DummySS $(_jsoncpp_dir)/include
VPATH += . ../.. ../../BroadcastChannel/soap ../../BroadcastChannel ../../DummySS

LDFLAGS += -lServiceShell $(commonlink) $(icelink) $(snmplink) -lboost_regex -lpthread -ldl -L$(_jsoncpp_dir)/libs -ljson
CXXFLAGS := -DWITH_ICESTORM

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

StreamSmithAdmin.h StreamSmithAdmin.cpp: $(_files2)
	@echo "compiling ICE file $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $< --output-dir .
