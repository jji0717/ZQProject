TOPDIR := ../../../

include $(TOPDIR)/build/defines.mk

SOLIB := ClibMan_web

OBJS := ContentReplicaEx.os MetaLib.os BasePage.os ClibMain.os ClibMan_web.os ContentDetail.os \
	    ShowContent.os ShowVolume.os stroprt.os 

_dir1 := ../../MetaLib
_dir2 := ../../ContentLib

_icefile1 := $(_dir1)/MetaLib.ICE
_icefile2 := $(_dir2)/ContentReplicaEx.ICE

_genfile1 := $(_dir1)/MetaLib.cpp $(_dir1)/MetaLib.h
_genfile2 := $(_dir2)/ContentReplicaEx.cpp $(_dir2)/ContentReplicaEx.h

GENFILES := $(_genfile1) $(_genfile2)

INCDIR += $(_dir1) $(_dir2)

VPATH += $(_dir1):$(_dir2):

LDFLAGS += -lZQCommon -lTianShanCommon -lTianShanIce $(icelink) 

include $(TOPDIR)/build/common.mk

$(_genfile1): $(_icefile1)
	@echo "generating ICE source $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) --output-dir $(_dir1) $<

$(_genfile2): $(_icefile2)
	@echo "generating ICE source $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) -I$(_dir1) --output-dir $(_dir2) $<


