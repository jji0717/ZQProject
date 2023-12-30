include  $(top_srcdir)/config/compileflags.mk
_files := ../CPE.ICE
GENFILES := ../CPE.cpp ../CPE.h
CXXFLAGS+=-I.. -I../common -I../Pacing -I../../CDNLib/CDNSS ../CPH_C2Propagation -I@_vstrm_inc_dir@  -I@_ctf_inc_dir@
CXXFLAGS+=-DCPH_CDN_EXPORTS -DLOGFMTWITHTID -DNAS -DDISABLE_PACING -D_LINUX -DEXCLUDE_VSTRM_API -D_VSTRM_DATA_TYPE_DEFINED -D_CTF_DATA_TYPE_DEFINED
 
VPATH=..:../common:../../CDNLib/CDNSS:@VPATHS@:
SOURCES:=CPE.cpp BaseCPH.cpp BaseFilter.cpp BaseGraph.cpp ConfigHelper.cpp CPH_CDN.cpp CPH_Cfg.cppa	\
	CStdFileIo.cpp CStdFileIoFactory.cpp FilesetTarget.cpp FTPMSClient.cpp FTPMSClientFactory.cpp \
	FTPPropagation.cpp FTPPropSource.cpp FTPSource.cpp LibBuffer.cpp LibQueue.cpp ProcessFactory.cpp \
	QueueBufMgr.cpp RTFProc.cpp SocketAPI.cpp CDNFileSetTarget.cpp  CIFSSource.cpp SourceFactory.cpp \
	TargetFac.cpp TargetFactoryI.cpp Utils.cpp VV2Parser.cpp VvxParser.cpp NormalizeSparseFile.cpp	\
	IndexFileParser.cpp SimpleXMLParser.cpp IdxFileParserEnvironment.cpp SelectPort.cpp ParseIndexFile.cpp	\
	C2PullSrc.cpp C2HttpClient.cpp HTTPClientFactory.cpp HTTPPropagation.cpp HTTPPropSource.cpp

AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libCPH_CDN.la
libCPH_CDN_la_SOURCES=$(SOURCES)
libCPH_CDN_la_LIBADD=
#libCPH_CDN_la_LDFLAGS=@lib_versioninfo@
AM_LDFLAGS=$(commonlink) $(icelink) $(snmplink) -L@_ctf_lib_dir@ -lCTF $(LDFLAGS)

.SECONDARY: $(GENFILES)
.PHONY:all clean test gent icetest

../CPE.h ../CPE.cpp: $(_files)
	@echo "compiling ICE file $< ......."
	@$(SLICE2CPP) $(SLICEFLAGS) $< --output-dir .
	cp CPE.{h,cpp} ../
clean-local:
	-rm -rf $(csbin_dir)/$(lib_LTLIBRARIES:%.la=%)*  $(cssyb_dir)/$(lib_LTLIBRARIES:%.la=%)*
tsflag:="so"
goal:=libCPH_CDN.so
include $(top_srcdir)/config/endpros.mk


