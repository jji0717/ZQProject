include  $(top_srcdir)/config/compileflags.mk

VPATH=@VPATHS@
AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=SystemShell
SystemShell_SOURCES=SystemShell.cpp
SystemShell_LDADD= 
AM_LDFLAGS=-lZQCommon -lSnmpManPkg -lsnmp $(LDFLAGS)
.PHONY:all clean testp
clean-local:
	-rm -rf $(bin_PROGRAMS).dbg rubbish $(csbin_dir)/$(bin_PROGRAMS)*  $(cssyb_dir)/$(bin_PROGRAMS)*
tsflag:="elf"
goal:=SystemShell
include $(top_srcdir)/config/endpros.mk

