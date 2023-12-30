include $(top_srcdir)/config/compileflags.mk

SOURCESALL:=ZQDaemon.cpp ConfigHelper.cpp ProvEventSink.cpp cspContentName.cpp CPCImpl.cpp  \
		ContentProvisionWrapper.cpp MCCSCfg.cpp ClusterCS.cpp CDNCSSvc.cpp D4Update.cpp
CXXFLAGS+=-I.. -I../../CPE -I../../common/vrep
CXXFLAGS+=-DCDNCS_SERVICE
VPATH=:@VPATHS@:
LDFLAGS+=-L..
AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=CDNCS
CDNCS_SOURCES=$(SOURCESALL)
CDNCS_LDADD=
AM_LDFLAGS=-lServiceShell -lContentStore -lvrep $(snmplink) $(commonlink) $(icelink)
clean-local:
	-rm -rf $(bin_PROGRAMS).dbg rubbish $(csbin_dir)/$(bin_PROGRAMS)*  $(cssyb_dir)/$(bin_PROGRAMS)*
tsflag:="elf"
goal:=$(bin_PROGRAMS)
include $(top_srcdir)/config/endpros.mk
