TOPDIR := ../../..
include $(TOPDIR)/build/defines.mk
TARGET := NativeCS

OBJS := ConfigHelper.o ZQDaemon.o FileSystemOp.o CPCImpl.o ProvWrapper.o \
        ProvEventSink.o cspNative.o NativeServiceConfig.o NativeService.o

#incdir += ../ ../../CPE
INCDIR := $(_tsdir)/common/  $(_prefix)/Common $(_tsdir)/Ice $(_ice_dir)/include  $(_tsdir)/common/vrep
INCDIR += ../ ../../CPE

include $(TOPDIR)/build/common.mk

LDFLAGS += $(cslink) $(shelllink) $(commonlink) $(icelink)
