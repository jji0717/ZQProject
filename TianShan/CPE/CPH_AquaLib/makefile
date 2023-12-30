TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

SOLIB := CPH_AquaLib

_files := ../CPE.ICE
GENFILES := ../CPE.cpp ../CPE.h

OBJS := CPH_AquaLib.os AquaLibHelper.os CPH_AquaLibCfg.os CPHAquaLibSession.os AquaUpdate.os\
		CPE.os BaseCPH.os BaseFilter.os BaseGraph.os ConfigHelper.os FTPMSClient.os \
	    CStdFileIo.os CStdFileIoFactory.os FilesetTarget.os FTPSource.os VV2Parser.os VvxParser.os \
	    CaptureInterface.os WPCapThread.os McastCapSrc.os MCastCapture.os MulticastWPCAP.os NICSelector.os StreamDataDumper.os \
	    LibBuffer.os LibQueue.os ProcessFactory.os QueueBufMgr.os RTFProc.os SocketAPI.os \
	    CIFSSource.os SourceFactory.os AquaLibTargetFac.os CDNFileSetTarget.os \
	    TargetFactoryI.os Utils.os NormalizeSparseFile.os \
	    IndexFileParser.os SimpleXMLParser.os IdxFileParserEnvironment.os SelectPort.os \
	    ParseIndexFile.os AquaFileIo.os AquaFileIoFactory.os AquaFileSetTarget.os MethodCostImpl.os AquaSource.os RTIRawSource.os

INCDIR += .. ../common ../CPH_Raw/CPH_Raw ../Pacing ../../CDNLib/CDNSS $(_vstrm_inc_dir) $(_ctf_inc_dir) $(_csi_inc_dir) $(TOPDIR)/Generic/CdmiFuse \
			$(_jsoncpp_dir)/include $(_curl_dir)/include $(TOPDIR)/TianShan/ContentStore $(TOPDIR)/TianShan/ComcastNGOD/CRM_A3Message
VPATH += ..:../common:../CPH_Raw/CPH_Raw:../../CDNLib/CDNSS $(TOPDIR)/TianShan/ComcastNGOD/CRM_A3Message

CXXFLAGS += -DLOGFMTWITHTID -DNAS -DDISABLE_PACING -D_LINUX -DUSE_SSLEAY -DUSE_OPENSSL -DEXCLUDE_VSTRM_API -D_VSTRM_DATA_TYPE_DEFINED -D_CTF_DATA_TYPE_DEFINED
LDFLAGS += $(commonlink) $(icelink) $(snmplink) -L$(_ctf_lib_dir) -lCTF -L$(_csi_dir)/Lib -lCSI -lpcap -L$(_curl_dir)/lib -lcurl -L$(_jsoncpp_dir)/libs -ljson -lAquaClient -lboost_thread-mt

include $(TOPDIR)/build/common.mk

../CPE.h ../CPE.cpp: $(_files)
	@echo "compiling ICE file $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) --output-dir .. $<

