TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

TARGET := RTICap

_files := ../CPE.ICE
GENFILES := ../CPE.cpp ../CPE.h

OBJS := CPE.o BaseCPH.o BaseFilter.o BaseGraph.o ConfigHelper.o RTICap.o\
	    CStdFileIo.o CStdFileIoFactory.o FilesetTarget.o \
	    LibBuffer.o LibQueue.o ProcessFactory.o QueueBufMgr.o RTFProc.o SocketAPI.o \
	    CDNFileSetTarget.o SourceFactory.o  McastCapSrc.o MCastCapture.o MulticastWPCAP.o\
	    StreamDataDumper.o WPCapThread.o  TargetFactoryI.o Utils.o  CaptureInterface.o

INCDIR += .. ../Pacing ../common $(_vstrm_inc_dir) $(_ctf_inc_dir)
VPATH += ..:../common 

CXXFLAGS += -DLOGFMTWITHTID -DNAS -DDISABLE_PACING -D_LINUX 
LDFLAGS += $(commonlink) $(icelink) -L$(_ctf_lib_dir) -lnl -lpcap -lCTF -lpthread

include $(TOPDIR)/build/common.mk

../CPE.h ../CPE.cpp: $(_files)
	@echo "compiling ICE file $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) --output-dir .. $<

