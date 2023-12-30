include  $(top_srcdir)/config/compileflags.mk

CXXFLAGS+=-I$(top_srcdir)/Common/HttpEngine -I$(top_srcdir)/Common/DataPostHouse
VPATH=:@VPATHS@:
AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=xmlshd
xmlshd_SOURCES=ConsoleCommand.cpp SimpleXMLParser.cpp XmlRPCService.cpp ConfigHelper.cpp ZQDaemon.cpp
xmlshd_LDADD=
AM_LDFLAGS=-lSnmpManPkg -lZQCommon -lServiceShell -lHttpEngine -lDataPostHouse -lboost_regex
clean-local:
	-rm -rf $(bin_PROGRAMS).dbg rubbish $(csbin_dir)/$(bin_PROGRAMS)*  $(cssyb_dir)/$(bin_PROGRAMS)*
tsflag:="elf"
goal:=xmlshd
include  $(top_srcdir)/config/endpros.mk
