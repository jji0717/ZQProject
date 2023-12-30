TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

TARGET := CDNCS

OBJS := ZQDaemon.o ConfigHelper.o \
        ProvEventSink.o cspContentName.o CPCImpl.o ContentProvisionWrapper.o \
        MCCSCfg.o ClusterCS.o CDNCSSvc.o D4Update.o NativeCS.o DispatchCS.o

INCDIR += .. ../../CPE ../../common/vrep

CXXFLAGS := -DCDNCS_SERVICE
LDFLAGS := -lServiceShell -lContentStore -lvrep $(snmplink) $(commonlink) $(icelink) -lboost_regex -lpthread

include $(TOPDIR)/build/common.mk
