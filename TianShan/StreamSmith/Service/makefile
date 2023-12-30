TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk  

TARGET := RtspProxy

OBJS := ZQDaemon.o ConfigHelper.o DataPostHouseService.o DialogCreator.o DialogType.o global.o \
		LscDialogImpl.o RtspDialog.o RtspSession.o RtspSessionMgr.o  StreamSmithSite.o StreamSmithService.o  Counter.o\
		FingerPrint.o rsaPrivateKey.o

INCDIR += .. ../LSCP/lscpLib $(TOPDIR)/Common/DataPostHouse \
		  $(TOPDIR)/Common/Rtsp/RtspParser $(TOPDIR)/Common/Rtsp/Utils  ../../common/snmp \
		  $(TOPDIR)/Common /opt/sdk/3rdPartyKits/openssl/include $(TOPDIR)/Generic/CdmiFuse/jsoncpp-src-0.5.0/include
VPATH += .. $(TOPDIR)/Common

CXXFLAGS += -DLOGFMTWITHTID -D_RTSP_PROXY -D_SUPPORT_LSC_PROTOCOL_ 

LDFLAGS += -lZQCommon -lTianShanIce $(snmplink) $(icelink) -lServiceShell -lRtspParser -lLSCP -lDataPostHouse -lssl -lcrypto -L$(_jsoncpp_dir)/libs -ljson -lpthread

include $(TOPDIR)/build/common.mk  

