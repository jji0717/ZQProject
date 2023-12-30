TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

SOLIB := SSM_OpenVBO

_files := ContentReplicaEx.ICE MetaLib.ICE SsmOpenVBO.ICE LAMFacade.ice StreamSmithAdmin.ICE

GENFILES := ContentReplicaEx.h ContentReplicaEx.cpp MetaLib.h MetaLib.cpp SsmOpenVBO.h SsmOpenVBO.cpp LAMFacade.h LAMFacade.cpp StreamSmithAdmin.h StreamSmithAdmin.cpp StreamIdx.h StreamIdx.cpp

OBJS := LAMFacade.os SsmOpenVBO.os StreamIdx.os StreamSmithAdmin.os ContentReplicaEx.os MetaLib.os\
	GetParameterHandler.os PauseHandler.os PlayHandler.os RequestHandler.os SetupHandler.os \
	StreamEventSinkI.os TeardownHandler.os ConfigHelper.os CRGSessionImpl.os CRGSessionManager.os \
	Environment.os FileSystemOp.os OpenVBOConfig.os OpenVBOResourceManager.os SelectionResourceManager.os \
	ssm_openVBO.os StreamersConfig.os stroprt.os ConfigHelper.os SelectionCommand.os

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

StreamIdx.h StreamIdx.cpp: SsmOpenVBO.ICE
	@echo "compiling ICE freeze $< ...."
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) --index \
		"SsmOpenVBO::StreamIdx,SsmOpenVBO::CRGSession,streamId"\
		StreamIdx $<


