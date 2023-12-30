TOPDIR := ../../../../..

include $(TOPDIR)/build/defines.mk

SOLIB := EGH_SOAP_CME

_files := CMEService.wsdl
GENFILES := CMEService.h CMESOAPC.cpp CMESOAPH.h CMESOAPStub.h CMESOAPClient.cpp \
            CMESOAPCMEServiceSoapBindingProxy.h CMEServiceSoapBinding.nsmap

OBJS := CMESOAPC.os CMESOAPClient.os CMESOAPClientImpl.os ConfigHelper.os EGH_SOAP_CME.os PlaylistEventSinkImpl.os \
        stdsoap2.os StreamEventSinkImpl.os

#INCDIR += .. ../../.. $(TOPDIR)/Common /opt/sdk/3rdPartyKits/expat/include $(TOPDIR)/TianShan/Ice \
          /opt/sdk/3rdPartyKits/Ice-3.2.2-linux/include $(TOPDIR)/TianShan/Shell/ZQSNMPManPkg/
INCDIR += .. ../../.. $(TOPDIR)/Common /opt/sdk/3rdPartyKits/expat/include $(TOPDIR)/TianShan/Ice \
          /opt/sdk/3rdPartyKits/ice/include $(TOPDIR)/TianShan/Shell/ZQSNMPManPkg/
VPATH += .:..

LDFLAGS += $(commonlink) $(icelink)

include $(TOPDIR)/build/common.mk

$(GENFILES): $(_files)
	@echo "compiling file $< ..."
	@wsdl2h -o CMEService.h $<
	@soapcpp2 -I/usr/share/gsoap/import -L -C -x -pCMESOAP CMEService.h
