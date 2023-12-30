TOPDIR := ../../../..

include $(TOPDIR)/build/defines.mk

TARGET := TransportsToQAMDevice

_files := ../../EdgeRM.ICE
GENFILES := EdgeRM.h EdgeRM.cpp

INCDIR += .:..
VPATH += .:..

OBJS := EdgeRM.o StreamLinkParser.o ConfigHelper.o

LDFLAGS += $(commonlink) $(icelink)

include $(TOPDIR)/build/common.mk

$(GENFILES): $(_files)
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $< --output-dir .
