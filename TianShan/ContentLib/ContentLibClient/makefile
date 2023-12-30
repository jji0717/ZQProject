TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

TARGET := ContentLibClient

OBJS := ContentReplicaEx.o MetaLib.o getopt.o ContentLibClient.o ContentLibGrammer.o ContentLibScanner.o   

_dir1 := ../../MetaLib
_dir2 := ..

_files := ContentLibScanner.l ContentLibGrammer.y
_icefile1 := $(_dir1)/MetaLib.ICE
_icefile2 := $(_dir2)/ContentReplicaEx.ICE

_genfile1 := $(_dir1)/MetaLib.cpp $(_dir1)/MetaLib.h  
_genfile2 := $(_dir2)/ContentReplicaEx.cpp $(_dir2)/ContentReplicaEx.h 

GENFILES := ContentLibScanner.cpp ContentLibGrammer.cpp $(_genfile1) $(_genfile2) 
		 
EXTRA_FILES := ContentLibGrammer.output

INCDIR += ../../MetaLib/ ..  

VPATH += ../../MetaLib/:..:

LDFLAGS += -lZQCommon -lTianShanCommon -lTianShanIce $(icelink) -lreadline -ltermcap

include $(TOPDIR)/build/common.mk

ContentLibGrammer.cpp: ContentLibGrammer.y
	@echo "compiling $< ..."
	$(Q)bison -dvtl -oContentLibGrammer.cpp $< 

ContentLibScanner.cpp: ContentLibScanner.l
	@echo "compiling $< ..."
	$(Q)flex -o$@ $<

$(_genfile1): $(_icefile1)
	@echo "generating ICE source $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) --output-dir $(_dir1) $<

$(_genfile2): $(_icefile2)
	@echo "generating ICE source $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) -I$(_dir1) --output-dir $(_dir2) $<



