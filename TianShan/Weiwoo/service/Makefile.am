include $(top_srcdir)/config/compileflags.mk

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

SOURCESALL:=StreamerToStreamLink.cpp WeiwooAdmin.cpp TsPathAdmin.cpp ServiceGroupDict.cpp ConfigHelper.cpp ZQDaemon.cpp \
	PathCommand.cpp PathFactory.cpp PathHelperMgr.cpp PathManagerImpl.cpp PathSvcEnv.cpp \
	ServiceGroupToStreamLink.cpp \
	SessionCommand.cpp SessionImpl.cpp SessionState.cpp SessionWatchDog.cpp \
	StorageDict.cpp StorageLinkToTicket.cpp StorageToStorageLink.cpp \
	StreamerDict.cpp StreamerReplicaUpdater.cpp StreamerToStorageLink.cpp StreamLinkToTicket.cpp \
	WeiwooFactory.cpp WeiwooService.cpp WeiwooSvcEnv.cpp 
        
#INCDIR += $(_dir1) $(_dir2)
CXXFLAGS+=-I$(_dir1) -I$(_dir2)
VPATH=:$(_dir1):$(_dir2):@VPATHS@:

CXXFLAGS+=-DWITH_ICESTORM -DEMBED_PATHSVC 

AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=Weiwoo
Weiwoo_SOURCES=$(SOURCESALL)
Weiwoo_LDADD=
AM_LDFLAGS=-lServiceShell $(icelink) $(snmplink) $(commonlink) -lSnmpLib

$(_genfiles1): $(word 1, $(_files))
	@echo "compiling ICE file $< ..."
	$(SLICE2CPP) $(SLICEFLAGS) --output-dir $(_dir1) $<
	$(SLICE2FREEZE) $(SLICEFLAGS) \
		--dict "TianShanIce::Transport::ServiceGroupDict,long,TianShanIce::Transport::ServiceGroup" \
		--dict-index "TianShanIce::Transport::ServiceGroupDict,id" ServiceGroupDict \
		--output-dir $(_dir1) $< 
	$(SLICE2FREEZE) $(SLICEFLAGS) \
		--dict "TianShanIce::Transport::StorageDict,string,TianShanIce::Transport::Storage"  \
		--dict-index "TianShanIce::Transport::StorageDict,netId,case-insensitive" StorageDict \
		--output-dir $(_dir1) $<
	$(SLICE2FREEZE) $(SLICEFLAGS) \
		--dict "TianShanIce::Transport::StreamerDict,string,TianShanIce::Transport::Streamer" \
		--dict-index "TianShanIce::Transport::StreamerDict,netId,case-insensitive" StreamerDict \
		--output-dir $(_dir1) $<
	$(SLICE2FREEZE) $(SLICEFLAGS) \
		--index "TianShanIce::Transport::StorageToStorageLink,TianShanIce::Transport::StorageLink,storageId" StorageToStorageLink \
		--output-dir $(_dir1) $<
	$(SLICE2FREEZE) $(SLICEFLAGS) \
		--index "TianShanIce::Transport::StreamerToStorageLink,TianShanIce::Transport::StorageLink,streamerId" StreamerToStorageLink \
		--output-dir $(_dir1) $<
	$(SLICE2FREEZE) $(SLICEFLAGS) \
		--index "TianShanIce::Transport::StreamerToStreamLink,TianShanIce::Transport::StreamLink,streamerId" StreamerToStreamLink \
		--output-dir $(_dir1) $<
	$(SLICE2FREEZE) $(SLICEFLAGS) \
	--index "TianShanIce::Transport::ServiceGroupToStreamLink,TianShanIce::Transport::StreamLink,servicegroupId" ServiceGroupToStreamLink \
		--output-dir $(_dir1) $<
	$(SLICE2FREEZE) $(SLICEFLAGS) \
		--index "TianShanIce::Transport::StorageLinkToTicket,TianShanIce::Transport::PathTicket,storageLinkIden" StorageLinkToTicket \
		--output-dir $(_dir1) $<
	$(SLICE2FREEZE) $(SLICEFLAGS) \
		--index "TianShanIce::Transport::StreamLinkToTicket,TianShanIce::Transport::PathTicket,streamLinkIden" StreamLinkToTicket \
		--output-dir $(_dir1) $<

$(_genfiles2): $(word 2, $(_files))
	$(SLICE2CPP) $(SLICEFLAGS) --output-dir $(_dir2) $<
clean-local:
	-rm -rf $(bin_PROGRAMS).dbg $(csbin_dir)/$(bin_PROGRAMS)*  $(cssyb_dir)/$(bin_PROGRAMS)*
tsflag:="elf"
goal:=Weiwoo
include $(top_srcdir)/config/endpros.mk
