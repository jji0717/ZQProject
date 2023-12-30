TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

TARGET := Weiwoo

_dir1 := ../../AccreditedPath
_dir2 := ..

_files := $(_dir1)/TsPathAdmin.ICE $(_dir2)/WeiwooAdmin.ICE

_genfiles1_ := TsPathAdmin.h TsPathAdmin.cpp \
	      ServiceGroupDict.h ServiceGroupDict.cpp \
	      StorageDict.h StorageDict.cpp \
	      StreamerDict.h StreamerDict.cpp \
	      StorageToStorageLink.h StorageToStorageLink.cpp \
	      StreamerToStorageLink.h StreamerToStorageLink.cpp \
	      StreamerToStreamLink.h StreamerToStreamLink.cpp \
	      ServiceGroupToStreamLink.h ServiceGroupToStreamLink.cpp \
	      StorageLinkToTicket.h StorageLinkToTicket.cpp \
	      StreamLinkToTicket.h StreamLinkToTicket.cpp 
_genfiles2_ := WeiwooAdmin.h WeiwooAdmin.cpp IdToSess.h IdToSess.cpp

_genfiles1 := $(addprefix $(_dir1)/, $(_genfiles1_))
_genfiles2 := $(addprefix $(_dir2)/, $(_genfiles2_))

GENFILES := $(_genfiles1) $(_genfiles2)

OBJS := ConfigHelper.o ZQDaemon.o \
	PathCommand.o PathFactory.o PathHelperMgr.o PathManagerImpl.o PathSvcEnv.o \
	ServiceGroupDict.o ServiceGroupToStreamLink.o \
	SessionCommand.o SessionImpl.o SessionState.o SessionWatchDog.o \
	StorageDict.o StorageLinkToTicket.o StorageToStorageLink.o \
	StreamerDict.o StreamerReplicaUpdater.o StreamerToStorageLink.o StreamerToStreamLink.o StreamLinkToTicket.o \
	TsPathAdmin.o WeiwooAdmin.o WeiwooFactory.o WeiwooService.o WeiwooSvcEnv.o 
        
INCDIR += $(_dir1) $(_dir2) ./../../common/snmp 

VPATH += $(_dir1):$(_dir2)

CXXFLAGS := -DWITH_ICESTORM -DEMBED_PATHSVC 
LDFLAGS := -lServiceShell $(icelink) $(snmplink) $(commonlink) -lpthread -ldl

include $(TOPDIR)/build/common.mk

$(_genfiles1): $(word 1, $(_files))
	@echo "compiling ICE file $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) --output-dir $(_dir1) $<
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
		--dict "TianShanIce::Transport::ServiceGroupDict,long,TianShanIce::Transport::ServiceGroup" \
		--output-dir $(_dir1) ServiceGroupDict $< 
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
		--dict "TianShanIce::Transport::StorageDict,string,TianShanIce::Transport::Storage"  \
		--output-dir $(_dir1) StorageDict $<
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
		--dict "TianShanIce::Transport::StreamerDict,string,TianShanIce::Transport::Streamer" \
		--output-dir $(_dir1) StreamerDict $<
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
		--index "TianShanIce::Transport::StorageToStorageLink,TianShanIce::Transport::StorageLink,storageId" StorageToStorageLink \
		--output-dir $(_dir1) $<
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
		--index "TianShanIce::Transport::StreamerToStorageLink,TianShanIce::Transport::StorageLink,streamerId" StreamerToStorageLink \
		--output-dir $(_dir1) $<
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
		--index "TianShanIce::Transport::StreamerToStreamLink,TianShanIce::Transport::StreamLink,streamerId" StreamerToStreamLink \
		--output-dir $(_dir1) $<
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
	--index "TianShanIce::Transport::ServiceGroupToStreamLink,TianShanIce::Transport::StreamLink,servicegroupId" ServiceGroupToStreamLink \
		--output-dir $(_dir1) $<
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
		--index "TianShanIce::Transport::StorageLinkToTicket,TianShanIce::Transport::PathTicket,storageLinkIden" StorageLinkToTicket \
		--output-dir $(_dir1) $<
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
		--index "TianShanIce::Transport::StreamLinkToTicket,TianShanIce::Transport::PathTicket,streamLinkIden" StreamLinkToTicket \
		--output-dir $(_dir1) $<

$(_genfiles2): $(word 2, $(_files))
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) --output-dir $(_dir2) $<
