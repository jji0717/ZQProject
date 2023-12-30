include  $(top_srcdir)/config/compileflags.mk

NEED_INCDIS:=.. ../LSCP/lscpLib $(top_srcdir)/Common/DataPostHouse $(top_srcdir)/Common/Rtsp/RtspParser $(top_srcdir)/Common/Rtsp/Utils
CXXFLAGS+=$(addprefix -I,$(NEED_INCDIS))
VPATH=:..:$(top_srcdir)/Common:@VPATHS@:
CXXFLAGS+=-DLOGFMTWITHTID -D_RTSP_PROXY -D_SUPPORT_LSC_PROTOCOL_
AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=RtspProxy
RtspProxy_SOURCES=ZQDaemon.cpp ConfigHelper.cpp DataPostHouseService.cpp DialogCreator.cpp DialogType.cpp global.cpp \
		LscDialogImpl.cpp RtspDialog.cpp RtspSession.cpp RtspSessionMgr.cpp  StreamSmithSite.cpp StreamSmithService.cpp  Counter.cpp
RtspProxy_LDADD=
AM_LDFLAGS=-lZQCommon -lTianShanIce $(snmplink) $(icelink) -lSnmpLib -lServiceShell -lRtspParser -lLSCP -lDataPostHouse -lssl $(LDFLAGS)
clean-local:
	-rm -rf $(bin_PROGRAMS).dbg rubbish $(csbin_dir)/$(bin_PROGRAMS)*  $(cssyb_dir)/$(bin_PROGRAMS)*
tsflag:="elf"
goal:=RtspProxy
include  $(top_srcdir)/config/endpros.mk

