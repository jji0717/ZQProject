TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

SOLIB := SSM_NGOD2

_files := StreamSmithAdmin.ICE ngod.ice LAMFacade.ice $(TOPDIR)/TianShan/ContentLib/ContentReplicaEx.ICE $(TOPDIR)/TianShan/MetaLib/MetaLib.ICE

GENFILES := LAMFacade.h LAMFacade.cpp MetaLib.h MetaLib.cpp ngod.cpp ngod.h ContentReplicaEx.h ContentReplicaEx.cpp StreamSmithAdmin.h StreamSmithAdmin.cpp \
            StreamIdx.h StreamIdx.cpp GroupIdx.h GroupIdx.cpp Streamer2Sess.h Streamer2Sess.cpp

OBJS := ConfigHelper.os FileSystemOp.os AnnounceRequest.os ClientRequest.os D5Update.os NgodConfig.os NgodDatabase.os NgodEnv.os NgodHelper.os NgodScheduler.os \
        NgodService.os NgodSession.os NgodSessionGetPara.os NgodSessionManager.os NgodSessionPause.os NgodSessionPlay.os \
        NgodSessionSetup.os NgodSessionTeardown.os RemoteAssetStack.os SelectionCommand.os SelectionResourceManager.os ServerResponse.os \
        SessionViewImpl.os SOPConfig.os ssm_ngod.os StreamEventSinker.os \
        ngod.os ContentReplicaEx.os StreamSmithAdmin.os MetaLib.os Streamer2Sess.os GroupIdx.os LAMFacade.os StreamIdx.os

INCDIR += $(TOPDIR)/Common $(TOPDIR)/TianShan/ComcastNGOD/ssm_ngod \
          $(TOPDIR)/TianShan/ComcastNGOD/ssm_ngod/Ice \
          $(TOPDIR)/Tianshan/Ice \
          $(_ice_slice_dir)/Ice \
          $(TOPDIR)/TianShan/common/vrep
			
VPATH += $(TOPDIR)/TianShan/ComcastNGOD/ssm_ngod/Ice/

CXXFLAGS += -DWITH_ICESTORM -ldl -lxx -I$(TOPDIR)/TianShan/ComcastNGOD/ssm_ngod/Ice/
LDFLAGS += -lZQCommon -lTianShanIce -lTianShanCommon $(snmplink) $(icelink) -lboost_regex -lvrep

include $(TOPDIR)/build/common.mk

ngod.h ngod.cpp: ngod.ice
	$(Q)mkdir -p Ice
	@echo "compiling ICE file $< ...."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) -I$(_ice_slice_dir)/Ice --output-dir ./Ice/ $<
	@echo "end of compiling ICE file $< ...."
LAMFacade.h LAMFacade.cpp: LAMFacade.ice
	$(Q)mkdir -p Ice
	@echo "compiling ICE file $< ...."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) -I$(_ice_slice_dir)/Ice -I$(TOPDIR)/TianShan/MetaLib --output-dir ./Ice/ $<
	@echo "end of compiling ICE file $< ...."
StreamSmithAdmin.h StreamSmithAdmin.cpp: StreamSmithAdmin.ICE
	$(Q)mkdir -p Ice
	@echo "compiling ICE file $< ...."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) -I$(_ice_slice_dir)/Ice -I$(TOPDIR)/TianShan/MetaLib --output-dir ./Ice/ $<
	@echo "end of compiling ICE file $< ...."
MetaLib.h MetaLib.cpp: $(TOPDIR)/TianShan/MetaLib/MetaLib.ICE
	$(Q)mkdir -p Ice
	@echo "compiling ICE file $< ...."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) -I$(_ice_slice_dir)/Ice -I$(TOPDIR)/TianShan/MetaLib --output-dir ./Ice/ $<
	@echo "end of compiling ICE file $< ...."
ContentReplicaEx.h ContentReplicaEx.cpp: $(TOPDIR)/TianShan/ContentLib/ContentReplicaEx.ICE
	$(Q)mkdir -p Ice
	@echo "compiling ICE file $< ...."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) -I$(_ice_slice_dir)/Ice -I$(TOPDIR)/TianShan/ContentLib/ -I$(TOPDIR)/TianShan/MetaLib --output-dir ./Ice/ $<
	@echo "end of compiling ICE file $< ...."
StreamIdx.h StreamIdx.cpp: ngod.ice
	$(Q)mkdir -p Ice
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) -I$(_ice_slice_dir)/Ice --output-dir ./Ice/ $<
	@echo "compiling ICE freeze $< ...."
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) -I$(_ice_slice_dir) -I$(_ice_slice_dir)/Ice -I$(TOPDIR)/TianShan/Ice -I$(TOPDIR)/TianShan/common --index "NGOD::StreamIdx,NGOD::NgodSession,mStreamSessId" StreamIdx --output-dir ./Ice ngod.ice
	@echo "end of compiling ICE freeze $< ...."
GroupIdx.h GroupIdx.cpp: ngod.ice
	$(Q)mkdir -p Ice
	@echo "compiling ICE freeze $< ...."
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) -I$(_ice_slice_dir) -I$(_ice_slice_dir)/Ice -I$(TOPDIR)/TianShan/Ice -I$(TOPDIR)/TianShan/common --index "NGOD::GroupIdx,NGOD::NgodSession,mGroupId" GroupIdx --output-dir ./Ice ngod.ice
Streamer2Sess.h Streamer2Sess.cpp: ngod.ice
	$(Q)mkdir -p Ice
	@echo "compiling ICE freeze $< ...."
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) -I$(_ice_slice_dir) -I$(_ice_slice_dir)/Ice -I$(TOPDIR)/TianShan/Ice -I$(TOPDIR)/TianShan/common --index "NGOD::Streamer2Sess,NGOD::NgodSession,mStreamerNetId" Streamer2Sess --output-dir ./Ice ngod.ice
