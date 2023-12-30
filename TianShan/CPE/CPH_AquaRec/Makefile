TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

SOLIB := CPH_AquaRec

_files := ../CPE.ICE
GENFILES := ../CPE.cpp ../CPE.h

OBJS := CPE.os AquaRecLeadSess.os AquaRecLeadSessCol.os AquaRecLeadSessColI.os AquaRecLeadSessFac.os\
			AquaRecVirtualSess.os AquaRecVirtualSessFac.os AquaRecWrapper.os base_64.os BaseCPHAquaRec.os\
			BaseGraph.os CdmiClientBase.os CDMIHttpClient.os ConfigHelper.os CPH_AquaRec.os\
			CPH_AquaRecCfg.os CURLClient.os hmac_sha1.os sha1.os TimerWatcher.os

INCDIR += $(TOPDIR)/TianShan/CPE  $(TOPDIR)/TianShan/CPE/common $(TOPDIR)/Generic/CdmiFuse $(TOPDIR)/Common ../CPH_nPVR \
	$(TOPDIR)/Generic/CdmiFuse/jsoncpp-src-0.5.0/include $(_curl_dir)/include 

VPATH += .:..:../common:../../../Generic/CdmiFuse:$(TOPDIR)/Common:../CPH_nPVR 

LDFLAGS += $(icelink) $(commonlink) $(snmplink) -L$(_jsoncpp_dir)/libs -ljson -ljson -L$(_curl_dir)/lib -lcurl

include $(TOPDIR)/build/common.mk

../CPE.h ../CPE.cpp: $(_files)
	@echo "compiling ICE file $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) --output-dir .. $<
