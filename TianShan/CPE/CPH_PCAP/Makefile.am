include  $(top_srcdir)/config/compileflags.mk
_files := ../CPE.ICE
GENFILES := ../CPE.cpp ../CPE.h
CXXFLAGS+=-I.. -I../common -I../Pacing  -I@_vstrm_inc_dir@  -I@_ctf_inc_dir@
CXXFLAGS+=-DCPH_PCAP_EXPORTS -DNAS -DDISABLE_PACING -D_LINUX
VPATH=..:../common:@VPATHS@:

SOURCES:=CPE.cpp BaseCPH.cpp BaseFilter.cpp BaseGraph.cpp ConfigHelper.cpp	\
		CPH_PCAP.cpp CPH_PCAPCfg.cpp CStdFileIo.cpp CStdFileIoFactory.cpp 	\
		FilesetTarget.cpp CaptureInterface.cpp LibBuffer.cpp LibQueue.cpp 	\
		ProcessFactory.cpp QueueBufMgr.cpp RTFProc.cpp SocketAPI.cpp CDNFileSetTarget.cpp 	\
		SourceFactory.cpp TargetFac.cpp NICSelector.cpp StreamDataDumper.cpp	\
		MulticastWPCAP.cpp McastCapSrc.cpp MCastCapture.cpp WPCapThread.cpp		\
		TargetFactoryI.cpp Utils.cpp

AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libCPH_PCAP.la
libCPH_PCAP_la_SOURCES=$(SOURCES)
libCPH_PCAP_la_LIBADD=
#libCPH_PCAP_la_LDFLAGS=@lib_versioninfo@

AM_LDFLAGS=$(commonlink) $(icelink) -lpcap -L@_ctf_lib_dir@ -lCTF $(LDFLAGS)

.SECONDARY: $(GENFILES)
.PHONY:all clean test gent icetest

../CPE.h ../CPE.cpp: $(_files)
	@echo "compiling ICE file $< ......."
	@$(SLICE2CPP) $(SLICEFLAGS) $< --output-dir .
	cp CPE.h CPE.cpp  ../
	cp CPE.h CPE.cpp  ../CPH_C2Propagation/
clean-local:
	-rm -rf  CPE.{h,cpp} $(csbin_dir)/$(lib_LTLIBRARIES:%.la=%)*  $(cssyb_dir)/$(lib_LTLIBRARIES:%.la=%)*
tsflag:="so"
goal:=libCPH_PCAP.so
include  $(top_srcdir)/config/endpros.mk

