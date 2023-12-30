TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

TARGET := DsmccCRG

_dir1 := .

_files := $(_dir1)/clientrequest.ice

GENFILES := $(addprefix $(_dir1)/, clientrequest.cpp clientrequest.h clientidx.cpp clientidx.h)

OBJS := ZQDaemon.o ConfigHelper.o clientrequest.o clientidx.o clientrequestsession.o \
				datadialog.o DsmccMsg.o environment.o gatewaycenter.o \
				gatewayconfig.o gatewayservice.o requestimpl.o sessdb.o
				
INCDIR += $(TOPDIR)/Common/HttpEngine $(TOPDIR)/Common/DataPostHouse  $(TOPDIR)/TianShan/StreamSmith/LSCP/lscpLib \
					$(TOPDIR)/TianShan/common/snmp

LDFLAGS +=  $(snmplink) $(commonlink) $(icelink) -lServiceShell -lDataPostHouse -lLSCP -ldl -lpthread

include $(TOPDIR)/build/common.mk  

$(_dir1))/clientrequest.h $(_dir1)/clientrequest.cpp: $(_files)
	@echo "compiling ICE file $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $< --output-dir $(dir $(_files))

$(_dir1)/clientidx.h $(_dir1)/clientidx.cpp: $(_files)
	@echo "generate ICE freeze file $< ..."
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
	--index \
	"TianShanIce::ClientRequest::ClientIdx, \
	TianShanIce::ClientRequest::Session,clientId,case-sensitive" \
	clientidx $< --output-dir $(dir $(_files))
	