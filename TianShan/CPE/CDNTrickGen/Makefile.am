include  $(top_srcdir)/config/compileflags.mk
neededincdir:=.. ../../../Common ../common ../Pacing ../../CDNLib/CDNSS ../CPH_C2Propagation ../CPH_CDN
prefixincdir:=$(addprefix -I,$(neededincdir))	
CXXFLAGS+=$(prefixincdir) -I@_vstrm_inc_dir@  -I@_ctf_inc_dir@

CXXFLAGS+=-DXML_STATIC -DLOGFMTWITHTID -DNAS -D_LINUX -DEXCLUDE_VSTRM_API -D_VSTRM_DATA_TYPE_DEFINED -D_CTF_DATA_TYPE_DEFINED
VPATH=..:../common:../CPH_CDN:../../CDNLib/CDNSS:@VPATHS@:
SOURCEST:=CDNTrickGen.cpp BaseFilter.cpp BaseGraph.cpp ConfigHelper.cpp CStdFileIo.cpp \
	CStdFileIoFactory.cpp CDNFileSetTarget.cpp EncodeConvert.cpp \
	FTPMSClient.cpp  FTPSource.cpp SelectPort.cpp CPH_Cfg.cpp \
	CIFSSource.cpp LibBuffer.cpp LibQueue.cpp \
	ProcessFactory.cpp RTFProc.cpp SocketAPI.cpp SourceFactory.cpp \
	TargetFac.cpp TargetFactoryI.cpp VV2Parser.cpp VvxParser.cpp \
	IndexFileParser.cpp SimpleXMLParser.cpp IdxFileParserEnvironment.cpp \
	C2PullSrc.cpp C2HttpClient.cpp HTTPClientFactory.cpp ParseIndexFile.cpp

AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=TrickGen 
TrickGen_SOURCES=$(SOURCEST)
TrickGen_LDADD=
AM_LDFLAGS=-lZQCommon $(snmplink) -L@_ctf_lib_dir@ -lCTF $(LDFLAGS) 

lofiles:=BaseFilter.lo BaseGraph.lo ConfigHelper.lo CStdFileIo.lo	\
	CStdFileIoFactory.lo CDNFileSetTarget.lo FTPMSClient.lo	\
	FTPSource.lo SelectPort.lo CIFSSource.lo LibBuffer.lo	\
	LibQueue.lo ProcessFactory.lo RTFProc.lo SocketAPI.lo	\
	SourceFactory.lo TargetFac.lo TargetFactoryI.lo VV2Parser.lo	\
	VvxParser.lo IndexFileParser.lo	\
	SimpleXMLParser.lo IdxFileParserEnvironment.lo	\
	SelectPort.lo C2PullSrc.lo C2HttpClient.lo HTTPClientFactory.lo ParseIndexFile.lo
$(lofiles:%.lo=%.o) : rubbish
rubbish :$(lofiles)
#	ls -lt $^
	touch rubbish
clean-local:
	-rm -rf $(bin_PROGRAMS).dbg rubbish $(csbin_dir)/$(bin_PROGRAMS)*  $(cssyb_dir)/$(bin_PROGRAMS)*
tsflag:="elf"
goal:=TrickGen
include  $(top_srcdir)/config/endpros.mk
