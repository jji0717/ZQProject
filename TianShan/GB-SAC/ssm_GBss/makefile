TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

SOLIB := SSM_GBss

_files := $(TOPDIR)/TianShan/StreamSmith/Service/StreamSmithAdmin.ICE SsmGBss.ICE MetaLib.ICE LAMFacade.ice ContentReplicaEx.ICE
          
GENFILES := ContentReplicaEx.h ContentReplicaEx.cpp LAMFacade.h LAMFacade.cpp MetaLib.h MetaLib.cpp SsmGBss.h SsmGBss.cpp StreamSmithAdmin.h StreamSmithAdmin.cpp StreamIdx.h StreamIdx.cpp

OBJS := Environment.os ConfigHelper.os CRGSessionImpl.os CRGSessionManager.os DescribeHandler.os GBssConfig.os GBssResourceManager.os \
        GetParameterHandler.os PauseHandler.os PlayHandler.os RemoteAssetStack.os RequestHandler.os SelectionCommand.os \
        SelectionResourceManager.os SetupHandler.os ssm_GBss.os StreamersConfig.os StreamEventSinkI.os stroprt.os \
        TeardownHandler.os StreamSmithAdmin.os ContentReplicaEx.os LAMFacade.os MetaLib.os StreamIdx.os SsmGBss.os
        
INCDIR += $(TOPDIR)/Common $(TOPDIR)/TianShan/StreamSmith \
          $(TOPDIR)/TianShan/StreamSmith/Service \
          $(TOPDIR)/Tianshan/Ice $(_ice_slice_dir)/Ice
			
CXXFLAGS += -DWITH_ICESTORM -ldl -lxx
LDFLAGS += -lZQCommon -lTianShanIce -lTianShanCommon $(snmplink) $(icelink) -lboost_regex -lsnmp

include $(TOPDIR)/build/common.mk

%.h %.cpp: %.ice
	@echo "compiling ICE file $< ...."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $<
%.h %.cpp: %.ICE
	@echo "compiling ICE file $< ...."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) -I$(_ice_slice_dir)/Ice $<
StreamSmithAdmin.h StreamSmithAdmin.cpp: $(TOPDIR)/TianShan/StreamSmith/Service/StreamSmithAdmin.ICE
	@echo "compiling ICE file $< ...."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) -I$(_ice_slice_dir)/Ice $<
	
StreamIdx.h StreamIdx.cpp: ./SsmGBss.ICE SsmGBss.cpp SsmGBss.h
	@echo "compiling ICE freeze $< ...."
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) -I$(_ice_slice_dir)/Ice -I$(TOPDIR)/tianshan/ice -I$(TOPDIR)/tianshan/common \
	 --index "SsmGBss::StreamIdx,SsmGBss::CRGSession,streamId" StreamIdx $<