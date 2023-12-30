TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

TARGET := C2FE

_dir1 := ../CRM_C2Locator
_files := C2Locator.ice

GENFILES := C2Locator.h C2Locator.cpp

OBJS := ConfigHelper.o ZQDaemon.o CPCImpl.o C2Locator.o ContentProvisionWrapper.o D4Update.o C2HttpClient.o \
        CDNHttpBridge.o ParseIndexFile.o SimpleXMLParser.o VV2Parser.o VvxParser.o \
		IdxFileParserEnvironment.o IndexFileParser.o SimpleXMLParser.o \
		C2FESvc.o CdnSsCommand.o CdnStreamerManager.o CdnEnv.o cspNative.o EmbedContentStore.o ProvEventSink.o

_dir2 := $(TOPDIR)/TianShan/ContentStore/MediaClusterCS

INCDIR += $(TOPDIR)/TianShan/StreamService/ServiceLib \
          $(TOPDIR)/TianShan/CDNLib/CDNSS/C2Pump \
          $(TOPDIR)/Common/HttpEngine \
          $(TOPDIR)/Common/libasync \
          $(TOPDIR)/Common/DataPostHouse \
          $(TOPDIR)/TianShan/CPE \
          $(TOPDIR)/TianShan/common/vrep \
          $(_vstrm_inc_dir) $(_ctf_inc_dir) \
	  $(_dir1) $(_dir2) \
	  $(TOPDIR)/TianShan/CPE/common \
	  $(TOPDIR)/TianShan/CPE/CPH_CDN \
          $(TOPDIR)/TianShan/common/ \
	  $(TOPDIR)/TianShan/CDNLib  $(TOPDIR)/Generic/CdmiFuse $(TOPDIR)/TianShan/OpenVBO \
	  $(TOPDIR)/Generic/CdmiFuse/jsoncpp-src-0.5.0/include $(_curl_dir)/include\
	  $(TOPDIR)/TianShan/CDNLib/CDNSS/

VPATH += $(_dir1):$(_dir2) $(TOPDIR)/TianShan/CPE/common $(TOPDIR)/TianShan/CDNLib \
         $(_vstrm_inc_dir) $(_ctf_inc_dir)  $(TOPDIR)/Generic/CdmiFuse $(TOPDIR)/TianShan/OpenVBO \
         $(TOPDIR)/TianShan/CDNLib/CDNSS/

CXXFLAGS := -DNAS -DDISABLE_PACING -D_LINUX -DEXCLUDE_VSTRM_API -DCDNCS_SERVICE -D_VSTRM_DATA_TYPE_DEFINED -D_CTF_DATA_TYPE_DEFINE
LDFLAGS := -lContentStore -lServiceShell -lStreamService -lC2Pump -lHttpEngine -lDataPostHouse -lvrep -lasync\
	      $(icelink) $(snmplink) $(commonlink) -lboost_regex -lnl -laio -liptc -L$(_ctf_lib_dir) -lCTF -L$(_curl_dir)/lib -lcurl -L$(_jsoncpp_dir)/libs -ljson -lssl -lcrypto -lboost_thread-mt -lboost_system-mt -lpthread -lip4tc

include $(TOPDIR)/build/common.mk

%.h %.cpp: %.ice
	@echo "compiling ICE file $< ..."
	@$(SLICE2CPP) $(SLICEFLAGS) $<
