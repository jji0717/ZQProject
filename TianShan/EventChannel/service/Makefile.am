include $(top_srcdir)/config/compileflags.mk
VPATH=@VPATHS@
VPATH+=:..: 

AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=EventChannel
EventChannel_SOURCES=ZQDaemon.cpp ConfigHelper.cpp EmbeddedIceStorm.cpp Sentinel.cpp EventChannelService.cpp
EventChannel_LDADD= 
AM_LDFLAGS=-lServiceShell $(snmplink) $(commonlink) $(icelink) -lIceBox $(LDFLAGS)
#all:
#	install -D ${bin_PROGRAMS} $(top_srcdir)/TianShan/bin64
clean-local:
	-rm -rf $(bin_PROGRAMS).dbg rubbish $(csbin_dir)/$(bin_PROGRAMS)*  $(cssyb_dir)/$(bin_PROGRAMS)*
tsflag:="elf"
goal:=$(bin_PROGRAMS)
include $(top_srcdir)/config/endpros.mk
