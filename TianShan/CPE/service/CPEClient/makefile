TOPDIR := ../../../..

include $(TOPDIR)/build/defines.mk

TARGET := CPEClient

_files := ../../CPE.ICE
GENFILES := CPE.h CPE.cpp ContentToProvision.h ContentToProvision.cpp

_dir1 := ../..
_dir2 := ../
INCDIR += $(_dir1)/PT_FtpServer $(_dir1) $(_dir2)
VPATH += $(_dir1)/PT_FtpServer:$(_dir1):$(_dir2)

OBJS := CPE.o ContentToProvision.o CECommon.o CmdLine.o CPEClient.o

LDFLAGS += $(commonlink) $(icelink)

include $(TOPDIR)/build/common.mk

$(GENFILES): $(_files)
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $< --output-dir ../../
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
	 --index "TianShanIce::ContentProvision::ContentToProvision,TianShanIce::ContentProvision::ProvisionSessionEx,contentKey" \
	ContentToProvision  $< --output-dir ../../
