TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

TARGET := EdgeRM

_dir1 := ..

_files := $(_dir1)/EdgeRM.ICE
GENFILES := $(addprefix $(_dir1)/, EdgeRM.cpp EdgeRM.h DeviceOfZONE.cpp DeviceOfZONE.h \
                                                OwnerOfAllocationEx.cpp OwnerOfAllocationEx.h \
                                                       QAMSessOfAllocationEx.cpp QAMSessOfAllocationEx.h RouteToChannel.cpp RouteToChannel.h)

OBJS := EdgeRM.o AllocationOwnerImpl.o AllocationState.o \
                                ClientCmds.o ConfigHelper.o D6Update.o definition.o DeviceOfZONE.o \
                                EdgeRMCmds.o EdgeRMEnv.o EdgeRMFactory.o EdgeRMImpl.o EdgeRMMsgHandler.o \
                                EdgeRMService.o ERMIClient.o ERRPMsg.o OwnerOfAllocationEx.o QAMSessOfAllocationEx.o \
                                R6Client.o S6Handler.o TripServerDialog.o TripServerService.o \
                                TripThreadRequest.o ZQDaemon.o ConfigHelper.o RouteToChannel.o

INCDIR += $(_dir1) ../.. $(TOPDIR)/Common $(TOPDIR)/TianShan/common/vrep $(TOPDIR)/Common/DataPostHouse $(TOPDIR)/Common/libbz $(TOPDIR)/Common/RtspLib $(TOPDIR)/Common/RtspLib/RtspServerLib

VPATH += ..

LDFLAGS += -lServiceShell  $(snmplink) $(commonlink) $(icelink) -lRtspServer  -lvrep -lDataPostHouse -L$(_openssl_dir)/lib -lssl -lpthread

include $(TOPDIR)/build/common.mk

$(_dir1)/EdgeRM.h $(_dir1)/EdgeRM.cpp: $(_files)
	@echo "compiling ICE file $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $< --output-dir $(dir $(_files))

$(_dir1)/DeviceOfZONE.h $(_dir1)/DeviceOfZONE.cpp: $(_files)
	@echo "generate ICE freeze file $< ..."
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
	--index \
	"TianShanIce::EdgeResource::DeviceOfZONE, \
	TianShanIce::EdgeResource::EdgeDeviceEx,deviceZone" \
	DeviceOfZONE $< --output-dir $(dir $(_files))

$(_dir1)/OwnerOfAllocationEx.h $(_dir1)/OwnerOfAllocationEx.cpp: $(_files)
	@echo "generate ICE freeze file $< ..."
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
	--index \
	"TianShanIce::EdgeResource::OwnerOfAllocationEx, \
	TianShanIce::EdgeResource::AllocationEx,ownerKey" \
	OwnerOfAllocationEx $< --output-dir $(dir $(_files))

$(_dir1)/QAMSessOfAllocationEx.h $(_dir1)/QAMSessOfAllocationEx.cpp: $(_files)
	echo "generate ICE freeze file $< ..."
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
	--index \
	"TianShanIce::EdgeResource::QAMSessOfAllocationEx, \
	TianShanIce::EdgeResource::AllocationEx,qamSessionId" \
	QAMSessOfAllocationEx $< --output-dir $(dir $(_files))
	
$(_dir1)/RouteToChannel.h $(_dir1)/RouteToChannel.cpp: $(_files)
	echo "generate ICE freeze file $< ..."
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
	--index \
	"TianShanIce::EdgeResource::RouteToChannel, \
	TianShanIce::EdgeResource::EdgeChannelEx,routeName" \
	RouteToChannel $< --output-dir $(dir $(_files))
	