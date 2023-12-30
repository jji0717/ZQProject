TOPDIR := ../../../..

include $(TOPDIR)/build/defines.mk

SOLIB := EGH_JMS

_files := Message.ICE
GENFILES := Message.h Message.cpp MessageRecord.h MessageRecord.cpp

OBJS := ConfigHelper.os EGH_JMS.os JMSEventHelper.os JMSMessageChannel.os JMSMessageTransporter.os Message.os MessageRecord.os

INCDIR += ../.. $(TOPDIR)/Generic/JndiClient

LDFLAGS := -lJndiClient -lZQCommon -lTianShanCommon -lTianShanIce $(icelink) -lboost_regex

include $(TOPDIR)/build/common.mk

Message.h Message.cpp: $(_files)
	$(Q)echo 'compiling ICE file $< ...'
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $<

MessageRecord.h MessageRecord.cpp: $(_files)
	$(Q)echo 'compiling ICE freeze file $< ...'
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
        --dict EventGateway::JMS::MessageRecord,long,EventGateway::JMS::Message MessageRecord $<
