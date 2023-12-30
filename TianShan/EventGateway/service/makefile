TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

TARGET := EventGateway

OBJS := ZQDaemon.o ConfigHelper.o \
        EventGwService.o EGConfig.o EventGateway.o GenEventSinkI.o StreamEventSinkI.o PluginHelper.o Sentinel.o

INCDIR += ..
VPATH += ..

CXXFLAGS += -DWITH_ICESTORM
LDFLAGS += $(snmplink) $(commonlink) $(icelink) -lServiceShell -lpthread

include $(TOPDIR)/build/common.mk
