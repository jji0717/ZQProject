include  $(top_srcdir)/config/compileflags.mk
_files := ../CPE.ICE
GENFILES := ../CPE.cpp ../CPE.h

CXXFLAGS+=-I.. -I../common -I../Pacing -I../CPH_CDN -I../../CDNLib/CDNSS  -I@_vstrm_inc_dir@  -I@_ctf_inc_dir@
CXXFLAGS+=-DDISABLE_PACING -DNAS -DEXCLUDE_VSTRM_API -D_LINUX -D_CTF_DATA_TYPE_DEFINED -D_VSTRM_DATA_TYPE_DEFINED
VPATH=..:../common:../../CDNLib/CDNSS:../CPH_CDN:@VPATHS@:

SOURCES:=CPE.cpp BaseCPH.cpp BaseFilter.cpp BaseGraph.cpp C2HttpClient.cpp ConfigHelper.cpp	\
		 CPH_C2Propagation.cpp CPH_C2PropagationCfg.cpp CStdFileIo.cpp CStdFileIoFactory.cpp \
		 FilesetTarget.cpp HTTPClientFactory.cpp HTTPPropagation.cpp HTTPPropSource.cpp \
		 IdxFileParserEnvironment.cpp IndexFileParser.cpp NormalizeSparseFile.cpp QueueBufMgr.cpp \
		 SimpleXMLParser.cpp TargetFactoryI.cpp VV2Parser.cpp VvxParser.cpp ParseIndexFile.cpp

AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libCPH_C2Propagation.la
libCPH_C2Propagation_la_SOURCES=$(SOURCES)
libCPH_C2Propagation_la_LIBADD=
#libCPH_C2Propagation_la_LDFLAGS=@lib_versioninfo@

AM_LDFLAGS=$(commonlink) $(icelink) -L@_ctf_lib_dir@  -lCTF  $(LDFLAGS)
.PHONY: all clean testP test version prebuild  preinstall lofiles
.SECONDARY:$(GENFILES)

lofiles:=CPE.lo BaseFilter.lo BaseGraph.lo C2HttpClient.lo BaseCPH.lo ConfigHelper.lo CStdFileIo.lo CStdFileIoFactory.lo FilesetTarget.lo  \
HTTPClientFactory.lo HTTPPropagation.lo HTTPPropSource.lo IdxFileParserEnvironment.lo IndexFileParser.lo NormalizeSparseFile.lo \
QueueBufMgr.lo SimpleXMLParser.lo TargetFactoryI.lo VV2Parser.lo VvxParser.lo ParseIndexFile.lo
$(lofiles): rubbish $(lofiles:%.lo=%.o)

rubbish :$(lofiles:%.lo=%.o) 
#	size $^
	touch rubbish

../CPE.h ../CPE.cpp: $(_files)
	@echo "compiling ICE file $< ......."
	@$(SLICE2CPP) $(SLICEFLAGS) $< --output-dir .
	cp CPE.h CPE.cpp ../
clean-local:
	-rm -rf  CPE.{h,cpp} $(csbin_dir)/$(lib_LTLIBRARIES:%.la=%)*  $(cssyb_dir)/$(lib_LTLIBRARIES:%.la=%)*
tsflag:="so"
goal:=libCPH_C2Propagation.so
include  $(top_srcdir)/config/endpros.mk


