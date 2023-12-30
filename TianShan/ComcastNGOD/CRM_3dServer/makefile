TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

SOLIB := CRM_3dServer

_files := ../../CPE/CPE.ICE
GENFILES := CPE.h CPE.cpp

OBJS := C3dServerEnv.os C3dServerMsgHandler.os \
        ConfigHelper.os CURLClient.os \
        sha1.os hmac_sha1.os C3dServerCommon.os \
        CdmiClientBase.os CDMIHttpClient.os \
        CPE.os base_64.os CRM_C3dServer.os

INCDIR += ./ $(TOPDIR)/Common/HttpEngine \
          $(TOPDIR)/Generic/CdmiFuse \
          $(TOPDIR)/Generic/CdmiFuse/jsoncpp-src-0.5.0/include \
          $(_curl_dir)/include \
          $(TOPDIR)/TianShan/common \
          $(TOPDIR)/Common \
          $(TOPDIR)/TianShan/CPE
          
VPATH += $(TOPDIR)/Generic/CdmiFuse

CXXFLAGS += -DWITH_ICESTORM

LDFLAGS += -lHttpEngine \
           -L$(_curl_dir)/lib -lcurl \
           -L$(_jsoncpp_dir)/libs -ljson \
           $(icelink) \
           $(commonlink) \
           $(snmplink) 
           
include $(TOPDIR)/build/common.mk

$(GENFILES): $(_files)
	@echo "compiling ICE file $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $< 
	