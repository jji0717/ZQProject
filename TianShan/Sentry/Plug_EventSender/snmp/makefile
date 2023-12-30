TOPDIR := ../../../..

include $(TOPDIR)/build/defines.mk

SOLIB := SnmpPlug

OBJS := ConfigHelper.os  snmpplug.os SnmpSender.os  

INCDIR += ../../EventSink   

LDFLAGS += -lZQCommon -lSnmpManPkg -lsnmp++

include $(TOPDIR)/build/common.mk




