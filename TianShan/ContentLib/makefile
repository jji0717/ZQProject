TOPDIR := ../..

include $(TOPDIR)/build/defines.mk

TARGET := ContentLib

_dir1 := ../MetaLib
_dir2 := ../ContentStore

_files1 := $(_dir1)/MetaLib.ICE
_files2 := ContentReplicaEx.ICE 
_files3 := $(_dir2)/ContentStore.ICE

_genfiles1 := $(_dir1)/MetaLib.cpp $(_dir1)/MetaLib.h ValueIdx.cpp ValueIdx.h TypeIdx.cpp TypeIdx.h
_genfiles2 := ContentReplicaEx.cpp ContentReplicaEx.h 
_genfiles3 := ContentStore.cpp ContentStore.h

GENFILES := $(_genfiles1) $(_genfiles2) $(_genfiles3)

OBJS := ContentReplicaEx.o ContentStore.o MetaLib.o  ValueIdx.o TypeIdx.o ContentReplicaEx.o \
		ContentLibEnv.o ContentLibFactory.o ContentLibImpl.o ContentLibRequest.o \
		ContentLibUtil.o ContentReplicaImpl.o ContentStoreReplicaImpl.o MetaLibCmds.o \
		MetaLibImpl.o MetaLibFactory.o MetaVolumeImpl.o SyncThread.o \
		ConfigHelper.o ZQDaemon.o ContentLibSvc.o


INCDIR += $(_dir1) 

VPATH += $(_dir1): 

CXXFLAGS += -DWITH_ICESTORM 
LDFLAGS += -lServiceShell $(commonlink) $(snmplink) $(icelink)  -lpthread

include $(TOPDIR)/build/common.mk

$(_genfiles1): $(_files1)
	@echo "generating ICE source $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) --output-dir $(_dir1) $< 
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
    --index \
    "TianShanIce::Repository::ValueIdx, TianShanIce::Repository::LibMetaValue, value, case-insensitive" \
	ValueIdx --output-dir ./ $<
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
    --index \
	"TianShanIce::Repository::TypeIdx, TianShanIce::Repository::LibMetaObject, type, case-sensitive" \
	TypeIdx --output-dir ./ $<

$(_genfiles2): $(_files2)
	@echo "generating ICE source $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) -I$(_dir1) --output-dir ./ $< 



$(_genfiles3): $(_files3)
	@echo "generating ICE source $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) --output-dir ./ $< 




