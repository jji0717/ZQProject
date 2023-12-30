TOPDIR := ../..

include $(TOPDIR)/build/defines.mk

TARGET := EventRuleEngine

OBJS := ConfigHelper.o ZQDaemon.o EventRERequest.o EventRuleEngine.o \
        EventSinkImpl.o EventRuleEngineSVC.o 


CXXFLAGS += -DWITH_ICESTORM
LDFLAGS += -lServiceShell $(snmplink) $(commonlink) $(icelink) -lpthread

include $(TOPDIR)/build/common.mk  

