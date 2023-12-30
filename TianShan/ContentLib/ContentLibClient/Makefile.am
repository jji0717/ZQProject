include  $(top_srcdir)/config/compileflags.mk
_dir1 := ../../MetaLib
_dir2 := ..
_files := ContentLibScanner.l ContentLibGrammer.y
_icefile1 := $(_dir1)/MetaLib.ICE
_icefile2 := $(_dir2)/ContentReplicaEx.ICE
_genfile1 := $(_dir1)/MetaLib.cpp $(_dir1)/MetaLib.h
_genfile2 := $(_dir2)/ContentReplicaEx.cpp $(_dir2)/ContentReplicaEx.h
GENFILES := ContentLibScanner.cpp ContentLibGrammer.cpp $(_genfile1) $(_genfile2)
EXTRA_FILES := ContentLibGrammer.output

CXXFLAGS+=-I../../MetaLib/ -I..
VPATH=:../../MetaLib/:..:@VPATHS@:
AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=ContentLibClient
ContentLibClient_SOURCES=ContentReplicaEx.cpp MetaLib.cpp getopt.cpp ContentLibClient.cpp ContentLibGrammer.cpp ContentLibScanner.cpp
ContentLibClient_LDADD=
AM_LDFLAGS=-lZQCommon -lTianShanCommon -lTianShanIce $(icelink) -lreadline -ltermcap $(LDFLAGS)

ContentLibGrammer.cpp: ContentLibGrammer.y
	@echo "compiling $< ...... "
	bison -dvtl -oContentLibGrammer.cpp $< 
ContentLibScanner.cpp: ContentLibScanner.l
	@echo "compiling $< ...... "
	flex -o$@ $<
(_genfile1): $(_icefile1)
	@echo "generating ICE source $< ...... "
	$(SLICE2CPP) $(SLICEFLAGS) --output-dir $(_dir1) $<
$(_genfile2): $(_icefile2)
	@echo "generating ICE source $< ...... "
	$(SLICE2CPP) $(SLICEFLAGS) -I$(_dir1) --output-dir $(_dir2) $<
clean-local:
	-rm -rf {ContentLibGrammer,ContentLibGrammer,ContentLibScanner}.{hpp,cpp,output} $(bin_PROGRAMS).dbg rubbish $(csbin_dir)/$(bin_PROGRAMS)*  $(cssyb_dir)/$(bin_PROGRAMS)*
tsflag:="elf"
goal:=ContentLibClient
include  $(top_srcdir)/config/endpros.mk
