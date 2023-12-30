TOPDIR := ../../../..

include $(TOPDIR)/build/defines.mk

SOLIB := EGH_SnmpTrap

OBJS := SnmpSender.os EGH_SnmpTrap.os ConfigHelper.os 

INCDIR += ../.. ../../../Sentry/EventSink

LDFLAGS += -lsnmp++

include $(TOPDIR)/build/common.mk

