TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

TARGET := GBVSS

OBJS := ZQDaemon.o ConfigHelper.o GBVSSConfig.o GBSession.o GBVSSEnv.o GBVSSSvc.o  GBVSSUtil.o

INCDIR += .. ../LSCP/lscpLib $(TOPDIR)/Common/DataPostHouse \
			$(TOPDIR)/Common/Rtsp/Utils  ../../common/snmp \
			$(TOPDIR)/TianShan/StreamService/StreamPumper \
			$(TOPDIR)/TianShan/StreamService/ServiceLib \
			$(TOPDIR)/TianShan/common \
			$(TOPDIR)/TianShan/ice \
			$(TOPDIR)/TianShan/include
			
VPATH += .. $(TOPDIR)/Common \
			$(TOPDIR)/TianShan/StreamService/ServiceLib \
			$(TOPDIR)/TianShan/StreamService/StreamPumper \
			$(TOPDIR)/TianShan/common \
			$(TOPDIR)/TianShan/ice \
			$(TOPDIR)/TianShan/include
			
CXXFLAGS += -DWITH_ICESTORM 

LDFLAGS += -lZQCommon -lTianShanIce $(snmplink) $(icelink) -lServiceShell -lDataPostHouse -lssl -lTianShanCommon -lStreamPumper -lpthread

include $(TOPDIR)/build/common.mk  