include $(top_srcdir)/config/compileflags.mk
CXXFLAGS+=-I..
CXXFLAGS+=-DWITH_ICESTORM
VPATH:=:../:@VPATHS@:

AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=EventGateway
EventGateway_SOURCES=ZQDaemon.cpp ConfigHelper.cpp EventGwService.cpp EGConfig.cpp EventGateway.cpp \
			GenEventSinkI.cpp PluginHelper.cpp Sentinel.cpp
EventGateway_LDADD=
AM_LDFLAGS=$(snmplink) $(commonlink) $(icelink) -lServiceShell

clean-local:
	-rm -rf $(bin_PROGRAMS).dbg rubbish $(csbin_dir)/$(bin_PROGRAMS)*  $(cssyb_dir)/$(bin_PROGRAMS)*
tsflag:="elf"
goal:=EventGateway
include $(top_srcdir)/config/endpros.mk
