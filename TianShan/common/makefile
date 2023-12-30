TOPDIR := ../..

include $(TOPDIR)/build/defines.mk

LIB := TianShanCommon

_files := ZqSentryIce.ICE TianShanUtils.ICE
GENFILES := ZqSentryIce.cpp ZqSentryIce.h TianShanUtils.cpp TianShanUtils.h

OBJS := EventChannel.o ZqSentryIce.o TianShanUtils.o TianShanDefines.o Sentinel.o \
        ZqAdapter.o HtmlTempl.o IceLog.o TimerWatchdog.o TianShanIceHelper.o

include $(TOPDIR)/build/common.mk

%.h %.cpp: %.ICE
	@echo "compiling ICE file $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $< 
