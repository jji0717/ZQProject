TOPDIR := ../../../..

include $(TOPDIR)/build/defines.mk

SOLIB := PHO_HttpSS

_dir1 := .. . 
OBJS := ConfigHelper.os Config.os pho_HttpSS.os C2StreamPHO.os

INCDIR += $(_dir1)

LDFLAGS += $(commonlink) $(icelink) 

include $(TOPDIR)/build/common.mk

