TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

TARGET := Sentry

_WebProxy  := HttpClient.o WebProxy.o 
_WebServer := LayoutConfig.o SentryHttpImpl.o SentryPages.o WebServer.o
_EventSink := LogPosition.o LogPositionI.o TimeConv.o \
              LiteralFunc.o LogMonitor.o LogParserManager.o MessageSource.o MsgSenderPump.o RegExHandler.o 
_Sentry    := ZQDaemon.o ConfigHelper.o \
              LogPaserManagement.o Neighborhood.o \
              SentryCommand.o SentryConfig.o SentryEnv.o SentryImpl.o \
              DiskMonitor.o SentryService.o

_Ntp       := ClockSync.o NTPClient.o NTPServer.o NTPUDPSocket.o NTPUtils.o SystemClock.o

_files := LogPosition.ICE

GENFILES := LogPosition.cpp LogPosition.h  

OBJS := $(_EventSink) $(_WebProxy) $(_WebServer) $(_Ntp) $(_Sentry)

_dir := ../EventSink
_dir2 := ../NTPSync   ../../common/snmp
INCDIR += .. $(TOPDIR)/Common/HttpEngine $(TOPDIR)/Common/DataPostHouse $(_dir) $(_dir2)

VPATH += ..:$(_dir):$(_dir2)

LDFLAGS += $(snmplink) $(commonlink) $(icelink) -lServiceShell -lHttpEngine -lDataPostHouse -lboost_regex -ldl -lpthread -lboost_system-mt

include $(TOPDIR)/build/common.mk

LogPosition.h LogPosition.cpp: $(_files)
	@echo "generating ICE source $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $< 


