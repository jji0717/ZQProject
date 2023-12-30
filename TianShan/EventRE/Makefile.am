include $(top_srcdir)/config/compileflags.mk

CXXFLAGS+=-DWITH_ICESTORM
VPATH=:@VPATHS@:
AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=EventRuleEngine
EventRuleEngine_SOURCES=ConfigHelper.cpp ZQDaemon.cpp EventRERequest.cpp EventRuleEngine.cpp	\
			EventSinkImpl.cpp EventRuleEngineSVC.cpp 
EventRuleEngine_LDADD=
AM_LDFLAGS=-lServiceShell $(snmplink) $(commonlink) $(icelink) $(LDFLAGS)

clean-local:
	-rm -rf $(bin_PROGRAMS).dbg rubbish
clean-local:
	-rm -rf $(bin_PROGRAMS).dbg rubbish $(csbin_dir)/$(bin_PROGRAMS)*  $(cssyb_dir)/$(bin_PROGRAMS)*
tsflag:="elf"
goal:=EventRuleEngine
include  $(top_srcdir)/config/endpros.mk

