TOPDIR := ../../../..

include $(TOPDIR)/build/defines.mk

SOLIB := EGH_MQTT

_files := Message.ICE
GENFILES := Message.h Message.cpp MessageRecord.h MessageRecord.cpp

MQTTPATH = $(_3rdparty_sdk)/MQTT
OBJS := ConfigHelper.os EGH_MQTT.os EGH_MQTT.os Message.os MessageRecord.os MQTTEventHelper.os MQTTMessageChannel.os MQTTMessageTransporter.os 
INCDIR += ../..
INCDIR += $(MQTTPATH)/include
LINKDIR += $(MQTTPATH)/lib
LDFLAGS :=  $(commonlink)  $(icelink) -lpaho-mqtt3c -lboost_regex

include $(TOPDIR)/build/common.mk

Message.h Message.cpp: $(_files)
	$(Q)echo 'compiling ICE file $< ...'
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $<

MessageRecord.h MessageRecord.cpp: $(_files)
	$(Q)echo 'compiling ICE freeze file $< ...'
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
		--dict EventGateway::MQTT::MessageRecord,long,EventGateway::MQTT::Message MessageRecord $<

