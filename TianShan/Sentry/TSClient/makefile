TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

SOLIB := TSClient

OBJS := SNMPOper.os VarCache.os SnmpUtil.os TSClient.os 

include $(TOPDIR)/build/common.mk

LDFLAGS += $(commonlink) -lsnmp 
