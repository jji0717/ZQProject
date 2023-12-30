TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

TARGET := DummySvc

_files := ../../StreamSmith/Service/StreamSmithAdmin.ICE

GENFILES := StreamSmithAdmin.h StreamSmithAdmin.cpp 

OBJS := DummySvc.o DummyStreamSmith.o StreamSmithAdmin.o DummySSCfg.o ConfigHelper.o ZQDaemon.o

INCDIR +=  $(TOPDIR)/common ../../DummySS $(TOPDIR)/TianShan/common  $(TOPDIR)/TianShan/ice

VPATH += ../../DummySS $(TOPDIR)/common

LDFLAGS += -lServiceShell $(commonlink) $(icelink) $(snmplink) -lpthread

include $(TOPDIR)/build/common.mk

StreamSmithAdmin.h StreamSmithAdmin.cpp: $(_files)
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $<
