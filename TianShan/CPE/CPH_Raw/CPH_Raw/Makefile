TOPDIR := ../../../..

include $(TOPDIR)/build/defines.mk

SOLIB := CPH_Raw

_files := ../../CPE.ICE
GENFILES := ../../CPE.cpp ../../CPE.h

OBJS := CPE.os BaseCPH.os BaseFilter.os BaseGraph.os CaptureInterface.os ConfigHelper.os \
	    	CPH_RTIRaw.os CPH_RTIRawCfg.os MCastCapture.os MulticastWPCAP.os RTIRawFileIo.os RTIRawFileIoFactory.os\
	    	RTIRawSource.os RTIRawTarget.os RTIRawTargetFac.os SourceFactory.os TargetFactoryI.os WPCapThread.os
	    

INCDIR += ../..  ../../common $(TOPDIR)/TianShan/CPE  $(TOPDIR)/TianShan/common
VPATH += .:../..:../../common:

include $(TOPDIR)/build/common.mk

CXXFLAGS += -DCPH_PCAP_EXPORTS -DNAS  -DDISABLE_PACING  -D_LINUX 
LDFLAGS += $(commonlink) $(icelink) -lpcap

../../CPE.h ../../CPE.cpp: $(_files)
	@echo "compiling ICE file $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) --output-dir ../.. $<


