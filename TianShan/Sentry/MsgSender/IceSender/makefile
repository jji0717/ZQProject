TOPDIR := ../../../..
include $(TOPDIR)/build/defines.mk
SOLIB := IceSender

OBJS := BaseSender.os ConfigHelper.os IcePlug.os IceSender.os

#incdir := ../MsgSender ../../EventSink   
INCDIR := $(INCDIR)
INCDIR += ../MsgSender ../../EventSink   
VPATH := ../MsgSender: 

include $(TOPDIR)/build/common.mk


LDFLAGS += $(commonlink) $(icelink) $(shelllink)


