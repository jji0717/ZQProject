TOPDIR := ../../../..

include $(TOPDIR)/build/defines.mk

LIB := C2Pump

VPATH += $(TOPDIR)/Generic/CdmiFuse $(TOPDIR)/TianShan/OpenVBO  $(TOPDIR)/Common  $(TOPDIR)/TianShan/ComcastNGOD/CRM_A3Message

OBJS := C2HttpLibAsyncBridge.o AioFile.o C2EventUpdater.o C2HttpHandler.o C2Session.o \
        C2SessionDataRunner.o C2SessionHelper.o SlabPool.o SlabPoolDefine.o \
        C2SessionManager.o C2StreamerEnv.o C2StreamerLib.o \
        C2StreamerService.o C2TransferInit.o CacheServerRequest.o \
        C2TransferSession.o C2TransferTerm.o PortManager.o \
        LocateSparseTsHeader.o C2TunnelBwmon.o \
		m3u8.o dummyhls.o IndexFileParser.o SimpleXMLParser.o \
		IdxFileParserEnvironment.o ReadClient.o ReadHttp.o ReadHttpManger.o HttpFetcher.o \
		AquaReader.o AquaUpdate.o RequestHandle.o LocateRequest.o GetRequest.o TransferDelete.o C2Request.o \
		DiskCache.o C2UdpResponseHandler.o C2AsyncUdpHandler.o C2SessionUdpControl.o auth5i.o  HLSContent.o  \
		hmac_md5.o C2SessionUdpQuery.o C2SessionStatDump.o C2TsParser.o 

#		CdmiFuseOps.o cachelayer.o base_64.o CURLClient.o hmac_sha1.o \
#		sha1.o CDMIHttpClient.o  

CXXFLAGS := -DEXCLUDE_VSTRM_API

VPATH += ..  $(TOPDIR)/TianShan/ComcastNGOD/CRM_A3Messag $(TOPDIR)/TianShan/ContentStore

INCDIR += .. $(TOPDIR)/Generic/CdmiFuse $(TOPDIR)/TianShan/OpenVBO $(TOPDIR)/Common/HttpEngine $(TOPDIR)/Common/libasync $(TOPDIR)/Common/DataPostHouse   \
		  $(TOPDIR)/TianShan/ComcastNGOD/CRM_A3Message $(TOPDIR)/TianShan/ContentStore $(TOPDIR)/TianShan/common \
		  $(TOPDIR)/TianShan/CDNLib $(TOPDIR)/Common/libasync \
		  $(TOPDIR)/Generic/CdmiFuse/jsoncpp-src-0.5.0/include $(_curl_dir)/include $(_vstrm_inc_dir) $(_ctf_inc_dir)

include $(TOPDIR)/build/common.mk

