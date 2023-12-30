include  $(top_srcdir)/config/compileflags.mk
_files := ../CPE.ICE
GENFILES := ../CPE.cpp ../CPE.h

CXXFLAGS+=-I.. -I../common -I../Pacing  -I@_vstrm_inc_dir@  -I@_ctf_inc_dir@
CXXFLAGS+=-DLOGFMTWITHTID -DNAS -DDISABLE_PACING -D_LINUX
VPATH=..:../common:@VPATHS@:

SOURCES:=CPE.cpp BaseCPH.cpp BaseFilter.cpp BaseGraph.cpp ConfigHelper.cpp RTICap.cpp	\
	CStdFileIo.cpp CStdFileIoFactory.cpp FilesetTarget.cpp LibBuffer.cpp LibQueue.cpp \
	ProcessFactory.cpp QueueBufMgr.cpp RTFProc.cpp SocketAPI.cpp CDNFileSetTarget.cpp \
	SourceFactory.cpp  McastCapSrc.cpp MCastCapture.cpp MulticastWPCAP.cpp	\
	StreamDataDumper.cpp WPCapThread.cpp  TargetFactoryI.cpp Utils.cpp  CaptureInterface.cpp

AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=RTICap
RTICap_SOURCES=$(SOURCES)
RTICap_LDADD=
AM_LDFLAGS=$(commonlink) $(icelink) -L@_ctf_lib_dir@ -lnl -lpcap  -lCTF  $(LDFLAGS)

.SECONDARY: $(GENFILES)
.PHONY:all clean test gent icetest

../CPE.h ../CPE.cpp: $(_files)
	@echo "compiling ICE file $< ......."
	@$(SLICE2CPP) $(SLICEFLAGS) $< --output-dir .
	cp CPE.h CPE.cpp  ../
clean-local:
	-rm -rf $(bin_PROGRAMS).dbg rubbish $(csbin_dir)/$(bin_PROGRAMS)*  $(cssyb_dir)/$(bin_PROGRAMS)*
tsflag:="elf"
goal:=RTICap
include $(top_srcdir)/config/endpros.mk

