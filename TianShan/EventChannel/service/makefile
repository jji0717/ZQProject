TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

TARGET := EventChannel

OBJS := ZQDaemon.o ConfigHelper.o EmbeddedIceStorm.o Sentinel.o EventChannelService.o 

INCDIR += ..  
VPATH += ..:

LDFLAGS += -lServiceShell $(snmplink) $(commonlink) $(icelink) -lIceBox -ldl -lpthread

include $(TOPDIR)/build/common.mk  



