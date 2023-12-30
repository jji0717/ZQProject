TOPDIR := ../../../..

include $(TOPDIR)/build/defines.mk

LIB := C2Streamer

VPATH += $(TOPDIR)/Generic/CdmiFuse $(TOPDIR)/TianShan/OpenVBO  $(TOPDIR)/Common  $(TOPDIR)/TianShan/ComcastNGOD/CRM_A3Message

OBJS := C2HttpLibAsyncBridge.o AioFile.o C2EventUpdater.o C2HttpHandler.o C2Session.o \
        C2SessionDataRunner.o C2SessionHelper.o \
        C2SessionManager.o C2StreamerEnv.o C2StreamerLib.o \
        C2StreamerService.o C2TransferInit.o \
        C2TransferSession.o C2TransferTerm.o PortManager.o \
        LocateSparseTsHeader.o C2TunnelBwmon.o \
		CdmiFuseOps.o cachelayer.o base_64.o CURLClient.o hmac_sha1.o \
		sha1.o CDMIHttpClient.o  auth5i.o HLSContent.o hmac_md5.o \
		m3u8.o dummyhls.o IndexFileParser.o SimpleXMLParser.o \
		IdxFileParserEnvironment.o ReadClient.o ReadHttp.o HttpFetcher.o \
		AquaReader.o AquaUpdate.o RequestHandle.o LocateRequest.o GetRequest.o TransferDelete.o C2Request.o

CXXFLAGS := -DEXCLUDE_VSTRM_API

VPATH += ..  $(TOPDIR)/TianShan/ComcastNGOD/CRM_A3Messag

INCDIR += .. $(TOPDIR)/Generic/CdmiFuse $(TOPDIR)/TianShan/OpenVBO $(TOPDIR)/Common/HttpEngine $(TOPDIR)/Common/libasync $(TOPDIR)/Common/DataPostHouse   \
		  $(TOPDIR)/TianShan/ComcastNGOD/CRM_A3Message \
		  $(TOPDIR)/Generic/CdmiFuse/jsoncpp-src-0.5.0/include $(_curl_dir)/include $(_vstrm_inc_dir) $(_ctf_inc_dir)

include $(TOPDIR)/build/common.mk

