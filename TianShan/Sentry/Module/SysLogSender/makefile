TOPDIR := ../../../..

include $(TOPDIR)/build/defines.mk

SOLIB := SysLogger

OBJS := ConfigHelper.os RemoteSyslog.os SysLogger.os 

INCDIR += ../../EventSink   

CXXFLAGS +=  -DCONFIG_NO_SNMP

LDFLAGS += -lZQCommon

include $(TOPDIR)/build/common.mk




