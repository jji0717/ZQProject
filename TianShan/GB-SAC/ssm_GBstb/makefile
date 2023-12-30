TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

SOLIB := SSM_GBstb

_files := ./SessionContext_ice.ICE

GENFILES := SessionContext_ice.h SessionContext_ice.cpp StreamIdx.h StreamIdx.cpp

OBJS := SessionContext_ice.os StreamIdx.os ConfigHelper.os ConnectService.os DescribeRequest.os Environment.os\
	Factory.os GetParamRequest.os OptionRequest.os PauseRequest.os PingHandler.os PlaylistEvent.os PlayRequest.os\
	RequestHandler.os SessionContextImpl.os SessionView.os SetupRequest.os ssm_GBstb.os StreamEvent.os stroprt.os TeardownRequest.os TsConfig.os WatchDog.os

INCDIR +=  $(TOPDIR)/Common $(TOPDIR)/TianShan/StreamSmith

CXXFLAGS += -DWITH_ICESTORM
LDFLAGS += -lTianShanIce -lTianShanCommon -lZQCommon $(icelink) -lboost_regex -lsnmp

include $(TOPDIR)/build/common.mk

%.h %.cpp: %.ice
	@echo "compiling ICE file $< ...."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $<
%.h %.cpp: %.ICE
	@echo "compiling ICE file $< ...."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $<

StreamIdx.h StreamIdx.cpp: SessionContext_ice.ICE
	@echo "compiling ICE freeze $< ...."
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
	--index "TianShanS1::StreamIdx, TianShanS1::SessionContext, streamID" StreamIdx $<
