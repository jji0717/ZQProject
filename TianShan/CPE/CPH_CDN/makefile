TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

SOLIB := CPH_CDN

_files := ../CPE.ICE
GENFILES := ../CPE.cpp ../CPE.h

OBJS := CPE.os BaseCPH.os BaseFilter.os BaseGraph.os ConfigHelper.os CPH_CDN.os CPH_Cfg.os \
	    CStdFileIo.os CStdFileIoFactory.os FilesetTarget.os \
	    FTPMSClient.os FTPMSClientFactory.os FTPPropagation.os FTPPropSource.os FTPSource.os \
	    LibBuffer.os LibQueue.os ProcessFactory.os QueueBufMgr.os RTFProc.os SocketAPI.os \
	    CDNFileSetTarget.os  CIFSSource.os SourceFactory.os TargetFac.os \
	    TargetFactoryI.os Utils.os VV2Parser.os VvxParser.os NormalizeSparseFile.os \
	    IndexFileParser.os SimpleXMLParser.os IdxFileParserEnvironment.os SelectPort.os \
	    ParseIndexFile.os C2PullSrc.os C2HttpClient.os HTTPClientFactory.os HTTPPropagation.os HTTPPropSource.os

INCDIR += .. ../common ../Pacing ../../CDNLib/CDNSS ../CPH_C2Propagation $(_vstrm_inc_dir) $(_ctf_inc_dir)
VPATH += ..:../common:../../CDNLib/CDNSS

CXXFLAGS += -DCPH_CDN_EXPORTS -DLOGFMTWITHTID -DNAS -DDISABLE_PACING -D_LINUX -DEXCLUDE_VSTRM_API -D_VSTRM_DATA_TYPE_DEFINED -D_CTF_DATA_TYPE_DEFINED
LDFLAGS += $(commonlink) $(icelink) $(snmplink) -L$(_ctf_lib_dir) -lCTF

include $(TOPDIR)/build/common.mk

../CPE.h ../CPE.cpp: $(_files)
	@echo "compiling ICE file $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) --output-dir .. $<

