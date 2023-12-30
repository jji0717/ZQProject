TOPDIR := ../..

include $(TOPDIR)/build/defines.mk

TARGET := DummySS

_files := ../StreamSmith/Service/StreamSmithAdmin.ICE

GENFILES := StreamSmithAdmin.h StreamSmithAdmin.cpp 

OBJS := DummySS.o InitInfoLinux.o DummyStreamSmith.o StreamSmithAdmin.o

INCDIR += ../..  ../../common $(TOPDIR)/TianShan/common  $(TOPDIR)/TianShan/ice /opt/sdk/3rdPartyKits/Ice-3.2.1-VC80-x86/include/IceUtil

LDFLAGS += $(commonlink) $(icelink)



include $(TOPDIR)/build/common.mk

StreamSmithAdmin.h StreamSmithAdmin.cpp: $(_files)
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $<
