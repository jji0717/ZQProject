include  $(top_srcdir)/config/compileflags.mk

VPATH=@VPATHS@
CXXFLAGS+=-I$(top_srcdir)/Common/HttpEngine  -I$(top_srcdir)/Common/DataPostHouse
CXXFLAGS +=  -DCONFIG_NO_SNMP
AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=HttpCRG
HttpCRG_SOURCES= ZQDaemon.cpp ConfigHelper.cpp HttpCRGConfig.cpp HttpCRG.cpp
HttpCRG_LDADD= 
AM_LDFLAGS=$(snmplink) $(commonlink) $(icelink) -lHttpEngine -lServiceShell -lDataPostHouse  -lSnmpLib -lboost_regex 
.PHONY:all clean testp
clean-local:
	-rm -rf $(bin_PROGRAMS).dbg rubbish $(csbin_dir)/$(bin_PROGRAMS)*  $(cssyb_dir)/$(bin_PROGRAMS)*
tsflag:="elf"
goal:=HttpCRG
include $(top_srcdir)/config/endpros.mk
