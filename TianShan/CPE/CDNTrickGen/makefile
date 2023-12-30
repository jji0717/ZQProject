TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

TARGET := TrickGen

OBJS := BaseFilter.o BaseGraph.o ConfigHelper.o \
    	CStdFileIo.o CStdFileIoFactory.o CDNFileSetTarget.o EncodeConvert.o \
	FTPMSClient.o  FTPSource.o \
        SelectPort.o CPH_Cfg.o CIFSSource.o CDNTrickGen.o \
	LibBuffer.o LibQueue.o ProcessFactory.o RTFProc.o SocketAPI.o \
	SourceFactory.o TargetFac.o TargetFactoryI.o \
	VV2Parser.o VvxParser.o \
	IndexFileParser.o SimpleXMLParser.o IdxFileParserEnvironment.o \
  EncodeConvert.o SelectPort.o C2PullSrc.o C2HttpClient.o HTTPClientFactory.o ParseIndexFile.o

INCDIR += .. ../../../Common ../common ../Pacing ../../CDNLib/CDNSS ../CPH_C2Propagation ../CPH_CDN $(_vstrm_inc_dir) $(_ctf_inc_dir)
VPATH += ..:../common:../CPH_CDN:../../CDNLib/CDNSS

CXXFLAGS += -DXML_STATIC -DLOGFMTWITHTID -DNAS -D_LINUX -DEXCLUDE_VSTRM_API -D_VSTRM_DATA_TYPE_DEFINED -D_CTF_DATA_TYPE_DEFINED
LDFLAGS += -lZQCommon $(snmplink) -L$(_ctf_lib_dir) -lCTF -ldl -lpthread

include $(TOPDIR)/build/common.mk

