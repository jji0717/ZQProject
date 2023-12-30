include  $(top_srcdir)/config/compileflags.mk

SUBDIRS:= . ContentClient MediaClusterCS BaseCS  A3CS 
CXXFLAGS+=-fPIC 
icefilesf = ContentStore.ICE
GENFILESF = ContentStore.cpp ContentStore.h ChildVolume.cpp ChildVolume.h FileOfVol.cpp FileOfVol.h

icefiless = CacheStore.ICE
GENFILESS = CacheStore.cpp CacheStore.h
SOURCESAALL =ContentStore.cpp ChildVolume.cpp FileOfVol.cpp CacheStore.cpp ContentCmds.cpp ContentFactory.cpp ContentImpl.cpp \
	    ContentState.cpp ContentStoreImpl.cpp VolumeInfoCache.cpp CacheStoreImpl.cpp \
	    CacheCmds.cpp CacheCounter.cpp CacheDomain.cpp CacheFactory.cpp CacheFolder.cpp

AUTOMAKE_OPTIONS = foreign
lib_LIBRARIES=libContentStore.a
libContentStore_a_SOURCES=$(SOURCESAALL)
AM_LDFLAGS=$(LDFLAGS)

.SECONDARY:$(GENFILESF) $(GENFILESS)
#${GENFILESF} : $(icefilesf)
ContentStore.cpp ContentStore.h ChildVolume.cpp ChildVolume.h FileOfVol.cpp FileOfVol.h : $(icefilesf)
	@echo "Compiling1 ICE file $< ......"
	@$(SLICE2CPP) $(SLICEFLAGS) $<
	@$(SLICE2FREEZE) $(SLICEFLAGS) \
	--index \
	"TianShanIce::Storage::ChildVolume,TianShanIce::Storage::VolumeEx,parentVolName,case-sensitive" \
	ChildVolume $<
	@$(SLICE2FREEZE) $(SLICEFLAGS) \
	--index \
	"TianShanIce::Storage::FileOfVol,TianShanIce::Storage::UnivContent,identVolume" \
	FileOfVol $<
clean-local:
	-rm -rf {CacheStore,ChildVolume,ContentStore,FileOfVol}.{h,cpp}
#${GENFILESS} : $(icefiless)
CacheStore.cpp CacheStore.h : CacheStore.ICE
	@echo "Compiling2 ICE file $< ..."
	$(SLICE2CPP) $(SLICEFLAGS) $<
tsflag:="ar"
goal:=$(lib_LIBRARIES)
include $(top_srcdir)/config/endpros.mk
