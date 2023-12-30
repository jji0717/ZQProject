TOPDIR := ../..

include $(TOPDIR)/build/defines.mk

TARGET := BcastChannel

_dir1  := .
_dir2  := ../DummySS

_files := $(_dir1)/BcastChannelEx.ICE 
_files2 := $(_dir2)/StreamSmithAdmin.ICE
GENFILES := $(addprefix $(_dir1)/, BcastChannelEx.cpp BcastChannelEx.h ChannelItemDict.cpp ChannelItemDict.h \
                                   PlaylistId2BcastPublishPoint.cpp PlaylistId2BcastPublishPoint.h \
                                   CtrlNum2ChannelItemAssoc.cpp CtrlNum2ChannelItemAssoc.h \
                                   BcastPublishPoint2ItemAssoc.cpp BcastPublishPoint2ItemAssoc.h \
                                   ChItem2ItemAssoc.h ChItem2ItemAssoc.cpp \
                                   mainChName2BcastPublishPoint.h mainChName2BcastPublishPoint.cpp \
                                   StreamSmithAdmin.h StreamSmithAdmin.cpp) 

OBJS := StreamSmithAdmin.o BcastChannelEx.o  ChannelItemDict.o CtrlNum2ChannelItemAssoc.o PlaylistId2BcastPublishPoint.o \
        BcastPublishPoint2ItemAssoc.o ChItem2ItemAssoc.o mainChName2BcastPublishPoint.o BroadCastChannelEnv.o \
        BcastChDef.o BcastChFactory.o BcastChRequest.o BcastPublisherImpl.o soapC.o stdsoap2.o soapClient.o \
        BcastPublishPointImpl.o BcastPurchaseImpl.o \
        BcastAppServiceImpl.o BroadcastChService.o ConfigHelper.o ZQDaemon.o ChannelItemAssocImpl.o\
        NVODChannelPublishPointImpl.o PlaylistEventSinkImpl.o MRTClient.o MRTStreamService.o BcastMRTProxy.o

INCDIR += . ../.. $(_dir1)/soap ../DummySS

VPATH += . .. $(_dir1)/soap $(_dir1) ../DummySS

CXXFLAGS := -DWITH_ICESTORM -DBCAST_MRT

LDFLAGS += -lServiceShell $(commonlink) $(icelink) $(snmplink) -lboost_regex -lpthread

include $(TOPDIR)/build/common.mk

$(_dir1)/BcastChannelEx.h $(_dir1)/BcastChannelEx.cpp: $(_files)
	@echo "compiling ICE file $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $< --output-dir $(dir $(_files))

$(_dir1)/ChannelItemDict.h $(_dir1)/ChannelItemDict.cpp: $(_files)
	@echo "generate ICE freeze file $< ..."
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
	--dict \
	"ZQBroadCastChannel::ChannelItemDict,string, TianshanIce::Application::Broadcast::ChannelItemEx" \
	ChannelItemDict $< --output-dir $(dir $(_files))

$(_dir1)/PlaylistId2BcastPublishPoint.h $(_dir1)/PlaylistId2BcastPublishPoint.cpp: $(_files)
	@echo "generate ICE freeze file $< ..."
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
	--index \
	"ZQBroadCastChannel::PlaylistId2BcastPublishPoint, TianshanIce::Application::Broadcast::BcastPublishPointEx,playlistId,case-insensitive" \
	PlaylistId2BcastPublishPoint $< --output-dir $(dir $(_files))

$(_dir1)/CtrlNum2ChannelItemAssoc.h $(_dir1)/CtrlNum2ChannelItemAssoc.cpp: $(_files)
	echo "generate ICE freeze file $< ..."
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
	--index \
	"ZQBroadCastChannel::CtrlNum2ChannelItemAssoc, TianshanIce::Application::Broadcast::ChannelItemAssoc,playlistCtrlNum" \
	CtrlNum2ChannelItemAssoc $< --output-dir $(dir $(_files))
	
$(_dir1)/BcastPublishPoint2ItemAssoc.h $(_dir1)/BcastPublishPoint2ItemAssoc.cpp: $(_files)
	echo "generate ICE freeze file $< ..."
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
	--index \
	"ZQBroadCastChannel::BcastPublishPoint2ItemAssoc, TianshanIce::Application::Broadcast::ChannelItemAssoc,bcastPPIdent" \
	BcastPublishPoint2ItemAssoc $< --output-dir $(dir $(_files))
	
$(_dir1)/ChItem2ItemAssoc.h $(_dir1)/ChItem2ItemAssoc.cpp: $(_files)
	echo "generate ICE freeze file $< ..."
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
	--index \
	"ZQBroadCastChannel::ChItem2ItemAssoc, TianshanIce::Application::Broadcast::ChannelItemAssoc,channelItemKey" \
	ChItem2ItemAssoc $< --output-dir $(dir $(_files))
	
$(_dir1)/mainChName2BcastPublishPoint.h $(_dir1)/mainChName2BcastPublishPoint.cpp: $(_files)
	echo "generate ICE freeze file $< ..."
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
	--index \
	"ZQBroadCastChannel::mainChName2BcastPublishPoint, TianshanIce::Application::Broadcast::BcastPublishPointEx,mainChName" \
	mainChName2BcastPublishPoint $< --output-dir $(dir $(_files))
	
$(_dir1)/StreamSmithAdmin.h $(_dir1)/StreamSmithAdmin.cpp: $(_files2)
	@echo "compiling ICE file $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $< --output-dir $(dir $(_files))
	
