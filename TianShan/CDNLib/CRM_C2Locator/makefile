TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

SOLIB := CRM_C2Locator

_files := C2Locator.ice LAMFacade.ice
GENFILES := C2Locator.h C2Locator.cpp SessionIdx.h SessionIdx.cpp LAMFacade.h LAMFacade.cpp

OBJS := C2Locator.os SessionIdx.os LAMFacade.os \
        C2Env.os C2Factory.os C2LocatorConf.os C2LocatorImpl.os ClientManager.os \
        CRM_C2Locator.os LocateRequestHandler.os ../SimpleXMLParser.os \
        TransferSessionImpl.os TransferDeleteRequestHandler.os TransferPortManager.os \
        ConfigHelper.os Text.os HttpClient.os CacheServerRequestHandle.os auth5i.os hmac_md5.os\
        
VPATH +=  $(TOPDIR)/TianShan/OpenVBO       

INCDIR += .. $(TOPDIR)/Common/HttpEngine $(TOPDIR)/Common $(TOPDIR)/TianShan/OpenVBO
INCDIR := $(TOPDIR)/Common/snmp $(INCDIR)
CXXFLAGS += -DWITH_ICESTORM
LDFLAGS += -lTianShanIce -lTianShanCommon -lZQCommon $(icelink) -lboost_regex $(snmplink)

include $(TOPDIR)/build/common.mk

%.h %.cpp: %.ice
	@echo "compiling ICE file $< ...."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $<

SessionIdx.h SessionIdx.cpp: C2Locator.ice
	@echo "compiling ICE freeze $< ...."
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) --index \
		"TianShanIce::SCS::SessionIdx,TianShanIce::SCS::TransferSession,sessKey,case-sensitive" \
		SessionIdx $<


