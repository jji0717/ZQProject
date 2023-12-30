include  $(top_srcdir)/config/compileflags.mk

_TSSDKCommon := ServiceShell.cpp ConfigHelper.cpp ZQDaemon.cpp
_CMECommon := CacheStorage.cpp
_CMESOAP := CMESOAPC.cpp CMESOAPClient.cpp CMESOAPServer.cpp LAMFacadeForCMEC.cpp LAMFacadeForCMEClient.cpp stdsoap2.cpp
CMEV2 := CMESvc.cpp CMECacheAlgorithm.cpp CMECfg.cpp CMELAMSOAPClient.cpp CMESOAPServiceImpl.cpp \
       CMESOAPServiceThread.cpp VSISCacheIO.cpp VSISConnFactory.cpp

SOURCESALL:=$(_TSSDKCommon) $(_CMECommon) $(_CMECommon) $(CMEV2)

CXXFLAGS+=-I.. -I../common -I./inc  -I./gSoapFiles  -I@_tianshan_sdk_inc_dir@  -I@_expat_dir@/include

CXXFLAGS+=-DLOGFMTWITHTID -DNAS -DDISABLE_PACING -D_LINUX
VPATH=..:../common:./gSoapFiles:$(_tianshan_sdk)/common:@VPATHS@:

AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=CMEV2
CMEV2_SOURCES=$(SOURCESALL)
CMEV2_LDADD=
AM_LDFLAGS=$(commonlink) -lServiceShell -lDataPostHouse $(LDFLAGS)

.PHONY:all clean test gent icetest

clean-local:
	-rm -rf $(bin_PROGRAMS).dbg rubbish $(csbin_dir)/$(bin_PROGRAMS)*  $(cssyb_dir)/$(bin_PROGRAMS)*
tsflag:="elf"

