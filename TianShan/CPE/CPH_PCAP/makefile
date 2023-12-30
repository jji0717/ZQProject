TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

SOLIB := CPH_PCAP

_files := ../CPE.ICE
GENFILES := ../CPE.cpp ../CPE.h

OBJS := CPE.os BaseCPH.os BaseFilter.os BaseGraph.os ConfigHelper.os CPH_PCAP.os CPH_PCAPCfg.os \
	    CStdFileIo.os CStdFileIoFactory.os FilesetTarget.os CaptureInterface.os\
	    LibBuffer.os LibQueue.os ProcessFactory.os QueueBufMgr.os RTFProc.os SocketAPI.os \
	    CDNFileSetTarget.os SourceFactory.os TargetFac.os NICSelector.os StreamDataDumper.os\
	    MulticastWPCAP.os McastCapSrc.os MCastCapture.os WPCapThread.os\
	    TargetFactoryI.os Utils.os 

INCDIR += .. ../common ../Pacing  $(_vstrm_inc_dir) $(_ctf_inc_dir)

VPATH += ..:../common: 

include $(TOPDIR)/build/common.mk

CXXFLAGS += -DCPH_PCAP_EXPORTS -DNAS -DDISABLE_PACING -D_LINUX 
LDFLAGS +=  $(commonlink) $(icelink) -lpcap -L$(_ctf_lib_dir) -lCTF

../CPE.h ../CPE.cpp: $(_files)
	@echo "compiling ICE file $< ..."
	@$(SLICE2CPP) $(SLICEFLAGS) --output-dir .. $<

