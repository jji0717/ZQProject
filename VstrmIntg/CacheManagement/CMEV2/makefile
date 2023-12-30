TOPDIR := ../../../

include $(TOPDIR)/build/defines.mk

TARGET := CMEV2

_TSSDKCommon := ConfigHelper.o ZQDaemon.o
_CMECommon := CacheStorage.o
_CMESOAP := CMESOAPC.o CMESOAPClient.o CMESOAPServer.o LAMFacadeForCMEC.o LAMFacadeForCMEClient.o stdsoap2.o
_CMEV2 := CMESvc.o CMECacheAlgorithm.o CMECfg.o CMELAMSOAPClient.o CMESOAPServiceImpl.o CMESOAPServiceThread.o \
	        VSISCacheIO.o

OBJS := ${_TSSDKCommon} ${_CMECommon} ${_CMESOAP} ${_CMEV2}

INCDIR += .. ./../common ./inc ./gSoapFiles $(TOPDIR)/Common $(_expat_dir)/include
VPATH += ..:./../common:./gSoapFiles:$(TOPDIR)/Common

LDFLAGS += -lZQCommon -lServiceShell $(snmplink)

include $(TOPDIR)/build/common.mk

