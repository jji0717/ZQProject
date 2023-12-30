TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

SOLIB := EventSender

OBJS := ConfigHelper.os BaseSender.os TextWriter.os \
	EventSender.os IceSender.os JmsSender.os MagSender.os StdAfx.os

INCDIR += ../EventSink $(TOPDIR)/Generic/JndiClient

CXXFLAGS += -D_ICE_INTERFACE_SUPPORT
LDFLAGS += $(commonlink) $(icelink) $(snmplink) -lJndiClient

include $(TOPDIR)/build/common.mk
