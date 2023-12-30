TOPDIR := ../../../..

include $(TOPDIR)/build/defines.mk

TARGET := MediaClusterCS
     
OBJS := ZQDaemon.o ConfigHelper.o \
        ProvEventSink.o cspContentName.o CPCImpl.o ContentProvisionWrapper.o \
        MCCSCfg.o ClusterCS.o MCCSSvc.o D4Update.o NativeCS.o DispatchCS.o
        
INCDIR += .. ../.. ../../../CPE ../../../common/vrep
VPATH  += ../..:..

LDFLAGS := -lServiceShell -lContentStore -lvrep $(snmplink) $(commonlink) $(icelink) -lboost_regex -lpthread

include $(TOPDIR)/build/common.mk
