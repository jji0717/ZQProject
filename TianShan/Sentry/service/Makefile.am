include  $(top_srcdir)/config/compileflags.mk

_WebProxy  := HttpClient.cpp WebProxy.cpp

_WebServer := LayoutConfig.cpp SentryHttpImpl.cpp SentryPages.cpp WebServer.cpp

_EventSink := LogPosition.cpp LogPositionI.cpp TimeConv.cpp LiteralFunc.cpp LogMonitor.cpp \
		LogParserManager.cpp MessageSource.cpp MsgSenderPump.cpp RegExHandler.cpp

_Sentry    := ZQDaemon.cpp ConfigHelper.cpp LogPaserManagement.cpp Neighborhood.cpp \
		SentryCommand.cpp SentryConfig.cpp SentryEnv.cpp SentryImpl.cpp \
		DiskMonitor.cpp SentryService.cpp

_Ntp       := ClockSync.cpp NTPClient.cpp NTPServer.cpp NTPUDPSocket.cpp NTPUtils.cpp SystemClock.cpp

_files := LogPosition.ICE

GENFILES := LogPosition.cpp LogPosition.h
SOURCESALL:=$(_EventSink) $(_WebProxy) $(_WebServer) $(_Ntp) $(_Sentry)

_dir := ../EventSink
_dir2 := ../NTPSync

CXXFLAGS+=-I.. -I$(top_srcdir)/Common/HttpEngine -I$(top_srcdir)/Common/DataPostHouse -I$(_dir) -I$(_dir2) -I../../common/snmp/
VPATH=:$(top_srcdir)/Common:..:$(_dir):$(_dir2):@VPATHS@:


AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=Sentry
Sentry_SOURCES=$(SOURCESALL)
Sentry_LDADD=
Sentry_LDFLAGS=
AM_LDFLAGS=$(snmplink) $(commonlink) $(icelink) -lServiceShell -lHttpEngine -lDataPostHouse -lboost_regex -lSnmpLib $(LDFLAGS)

lofiles:=HttpClient.lo
$(lofiles:%.lo=%.o) : rubbish 
rubbish :$(lofiles)
	touch rubbish

LogPosition.h LogPosition.cpp: $(_files)
	@echo "generating ICE source $< ......"
	@$(SLICE2CPP) $(SLICEFLAGS) $< 
clean-local:
	-rm -rf LogPosition.{h,cpp} $(bin_PROGRAMS).dbg $(csbin_dir)/$(bin_PROGRAMS)*  $(cssyb_dir)/$(bin_PROGRAMS)*
tsflag:="elf"
goal:=Sentry
include  $(top_srcdir)/config/endpros.mk
