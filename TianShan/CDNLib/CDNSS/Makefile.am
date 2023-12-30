include  $(top_srcdir)/config/compileflags.mk
#SUBDIRS=../../StreamService/ServiceLib
dir1p:=../CRM_C2Locator
icefiles:=../CRM_C2Locator/C2Locator.ice
GENFILES:=C2Locator.h C2Locator.cpp

SOURCESALL:=C2Locator.cpp ConfigHelper.cpp ZQDaemon.cpp CPCImpl.cpp ContentProvisionWrapper.cpp \
	D4Update.cpp C2HttpClient.cpp IdxFileParserEnvironment.cpp ParseIndexFile.cpp \
	SimpleXMLParser.cpp VV2Parser.cpp VvxParser.cpp CdnEnv.cpp CdnSsCommand.cpp \
	CdnStreamerManager.cpp CdnSvc.cpp cspNative.cpp EmbedContentStore.cpp \
	IndexFileParser.cpp ProvEventSink.cpp vstrmIndexParser.cpp

dir2p:=$(top_srcdir)/TianShan/ContentStore/MediaClusterCS

NEEDINCDIRS:=$(top_srcdir)/TianShan/StreamService/ServiceLib $(top_srcdir)/TianShan/CDNLib/CDNSS/C2Streamer $(top_srcdir)/Common/HttpEngine $(top_srcdir)/Common/DataPostHouse $(top_srcdir)/TianShan/CPE $(top_srcdir)/TianShan/common/vrep $(_vstrm_inc_dir) $(_ctf_inc_dir) $(dir1p) $(dir2p) $(top_srcdir)/TianShan/CPE/common $(top_srcdir)/TianShan/CPE/CPH_CDN $(top_srcdir)/TianShan/CDNLib 

ADDPREFIXINCDIR:=$(addprefix -I,$(NEEDINCDIRS))
	
CXXFLAGS+=$(ADDPREFIXINCDIR)

VPATH=:$(dir1p):$(dir2p):$(top_srcdir)/TianShan/CPE/common:$(top_srcdir)/TianShan/CDNLib/CDNSS:$(top_srcdir)/TianShan/CDNLib:@_vstrm_inc_dir@:@_ctf_inc_dir@:@VPATHS@:

CXXFLAGS+=-DNAS -DDISABLE_PACING -D_LINUX -DEXCLUDE_VSTRM_API -DCDNCS_SERVICE -D_VSTRM_DATA_TYPE_DEFINED -D_CTF_DATA_TYPE_DEFINE
LDFLAGS+=-L../../ContentStore
AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=CDNSS
CDNSS_SOURCES=$(SOURCESALL)
CDNSS_LDADD=
AM_LDFLAGS=-lContentStore -lServiceShell -lStreamService -lC2Streamer -lHttpEngine -lDataPostHouse -lvrep -lSnmpLib -lSnmpLib $(icelink) $(snmplink) $(commonlink) -lboost_regex -lnl -laio -liptc  -L@_ctf_lib_dir@ -lCTF
lofiles:=ZQDaemon.lo ConfigHelper.lo C2Locator.lo SimpleXMLParser.lo CPCImpl.lo ContentProvisionWrapper.lo D4Update.lo ProvEventSink.lo
$(lofiles:%.lo=%.o) : rubbish
rubbish : $(lofiles)
	touch rubbish
 
C2Locator.h C2Locator.cpp : $(icefiles)
	@echo "compiling ICE file  $< ...... "
	$(SLICE2CPP) $(SLICEFLAGS) $< --output-dir .
clean-local:
	-rm -rf $(bin_PROGRAMS).dbg rubbish $(csbin_dir)/$(bin_PROGRAMS)*  $(cssyb_dir)/$(bin_PROGRAMS)*
tsflag:="elf"
goal:=CDNSS
include $(top_srcdir)/config/endpros.mk
