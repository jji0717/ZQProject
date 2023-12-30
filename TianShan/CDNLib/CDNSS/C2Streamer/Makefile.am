include $(top_srcdir)/config/compileflags.mk

CXXFLAGS+=-fPIC  -I$(top_srcdir)/Common/HttpEngine  -I$(top_srcdir)/Common/DataPostHouse
AUTOMAKE_OPTIONS=foreign
lib_LIBRARIES=libC2Streamer.a
libC2Streamer_a_SOURCES=AioFile.cpp C2EventUpdater.cpp C2HttpHandler.cpp C2Session.cpp \
			C2SessionDataRunner.cpp C2SessionHelper.cpp \
			C2SessionManager.cpp C2StreamerEnv.cpp C2StreamerLib.cpp \
			C2StreamerService.cpp C2TransferInit.cpp \
			C2TransferSession.cpp C2TransferTerm.cpp PortManager.cpp \
			LocateSparseTsHeader.cpp C2TunnelBwmon.cpp
libC2Streamer_a_LIBADD=
#libC2Streamer_a_LDFLAGS=
AM_LDFLAGS=
tsflag:="ar"
goal:=$(lib_LIBRARIES)
include  $(top_srcdir)/config/endpros.mk

