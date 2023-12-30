include  $(top_srcdir)/config/compileflags.mk

_dir1 := ../MetaLib
_dir2 := ../ContentStore
_files1 := $(_dir1)/MetaLib.ICE
_files2 := ContentReplicaEx.ICE
_files3 := $(_dir2)/ContentStore.ICE

_genfiles1 := $(_dir1)/MetaLib.cpp $(_dir1)/MetaLib.h ValueIdx.cpp ValueIdx.h TypeIdx.cpp TypeIdx.h
_genfiles2 := ContentReplicaEx.cpp ContentReplicaEx.h
_genfiles3 := ContentStore.cpp ContentStore.h

GENFILES := $(_genfiles1) $(_genfiles2) $(_genfiles3)

SOURCESALL:=ContentStore.cpp ContentReplicaEx.cpp MetaLib.cpp ValueIdx.cpp 	\
		TypeIdx.cpp ContentLibEnv.cpp ContentLibFactory.cpp 	\
		ContentLibImpl.cpp ContentLibRequest.cpp ContentLibUtil.cpp ContentReplicaImpl.cpp 	\
		ContentStoreReplicaImpl.cpp MetaLibCmds.cpp MetaLibImpl.cpp MetaLibFactory.cpp 	\
		MetaVolumeImpl.cpp SyncThread.cpp ConfigHelper.cpp ZQDaemon.cpp ContentLibSvc.cpp

CXXFLAGS+=-I$(_dir1)
VPATH=:$(_dir1):@VPATHS@:
CXXFLAGS+=-DWITH_ICESTORM

AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=ContentLib
ContentLib_SOURCES=$(SOURCESALL)
ContentLib_LDADD=
AM_LDFLAGS=-lServiceShell $(commonlink) $(snmplink) $(icelink) $(LDFLAGS)

$(_genfiles1): $(_files1)
	@echo "generating ICE source $< ...... "
	$(SLICE2CPP) $(SLICEFLAGS) --output-dir $(_dir1) $<
	$(SLICE2FREEZE) $(SLICEFLAGS)	\
	--index	\
	"TianShanIce::Repository::ValueIdx, TianShanIce::Repository::LibMetaValue, value, case-insensitive"	\
	ValueIdx --output-dir ./ $<
	$(SLICE2FREEZE) $(SLICEFLAGS)	\
	--index	\
	"TianShanIce::Repository::TypeIdx, TianShanIce::Repository::LibMetaObject, type, case-sensitive"	\
	TypeIdx --output-dir ./ $<
$(_genfiles2): $(_files2)
	@echo "generating ICE source $< ...... "
	$(SLICE2CPP) $(SLICEFLAGS) -I$(_dir1) --output-dir ./ $< 
$(_genfiles3): $(_files3)
	@echo "generating ICE source $< ...... "
	$(SLICE2CPP) $(SLICEFLAGS) --output-dir ./ $<
ContentStore.o : rubbish
rubbish : ContentStore.lo	
	touch rubbish
clean-local:
	-rm -rf {ContentReplicaEx,TypeIdx,ValueIdx,}.{h,cpp} $(bin_PROGRAMS).dbg rubbish $(csbin_dir)/$(bin_PROGRAMS)*  $(cssyb_dir)/$(bin_PROGRAMS)*
tsflag:="elf"
goal:=ContentLib
include  $(top_srcdir)/config/endpros.mk
