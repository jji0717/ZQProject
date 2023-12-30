TOPDIR := ../..

include $(TOPDIR)/build/defines.mk

LIB := ContentStore

_files := ContentStore.ICE
GENFILES := ContentStore.cpp ContentStore.h ChildVolume.cpp ChildVolume.h FileOfVol.cpp FileOfVol.h

_files2 := CacheStore.ICE
GENFILES2 := CacheStore.cpp CacheStore.h

OBJS := $(patsubst %.cpp,%.o,$(filter %.cpp, $(GENFILES))) \
		ContentCmds.o ContentFactory.o ContentImpl.o ContentState.o ContentStoreImpl.o VolumeInfoCache.o \
		CacheStoreImpl.o CacheStore.o CacheCmds.o CacheCounter.o CacheDomain.o CacheFactory.o CacheFolder.o 

include $(TOPDIR)/build/common.mk

$(GENFILES): $(_files)
	@echo "compiling ICE file $< ..."
	@$(SLICE2CPP) $(SLICEFLAGS) $<
	@$(SLICE2FREEZE) $(SLICEFLAGS) \
			--index \
			"TianShanIce::Storage::ChildVolume,TianShanIce::Storage::VolumeEx,parentVolName,case-sensitive" \
			ChildVolume $<
	@$(SLICE2FREEZE) $(SLICEFLAGS) \
			--index \
			"TianShanIce::Storage::FileOfVol,TianShanIce::Storage::UnivContent,identVolume" \
			FileOfVol $<
			
$(GENFILES2): $(_files2)
	@echo "compiling ICE file $< ..."
	@$(SLICE2CPP) $(SLICEFLAGS) $<			
