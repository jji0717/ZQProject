TOPDIR := ../../../..

include $(TOPDIR)/build/defines.mk
SOLIB := EGH_SMS

OBJS := ConfigHelper.os EGH_SMS.os EventHandle.os SmsCommon.os SmsControl.os

#incdir := ../..   
INCDIR := $(INCDIR)
INCDIR += ../..   

CXXFLAGS+= -DEGH_SMS_EXPORTS
LDFLAGS += $(commonlink) $(icelink) -lSnmpManPkg 

include $(TOPDIR)/build/common.mk



