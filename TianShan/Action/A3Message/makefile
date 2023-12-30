TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

SOLIB := Action_A3Message

OBJS := A3Call.os A3HttpReq.os A3Message.os HttpClient.os 

INCDIR += ../../EventRE/

CXXFLAGS := -DWITH_ICESTORM

LDFLAGS := $(commonlink) $(icelink)   

include $(TOPDIR)/build/common.mk
