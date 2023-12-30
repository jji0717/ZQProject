include  $(top_srcdir)/config/compileflags.mk

AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=TsHammer
TsHammer_SOURCES=AssetPool.cpp ScriptParser.cpp Hammer.cpp HammerSession.cpp HammerMonitor.cpp main.cpp
TsHammer_LDADD=
TsHammer_LDFLAGS=
AM_LDFLAGS=-lZQCommon -lboost_regex -lcurses
clean-local:
	-rm -rf $(bin_PROGRAMS).dbg rubbish $(csbin_dir)/$(bin_PROGRAMS)*  $(cssyb_dir)/$(bin_PROGRAMS)*
tsflag:="elf"
goal:=TsHammer
include $(top_srcdir)/config/endpros.mk

