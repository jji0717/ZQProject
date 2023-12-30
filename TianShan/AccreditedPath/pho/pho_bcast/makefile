TOPDIR := ../../../..

include $(TOPDIR)/build/defines.mk

SOLIB := PHO_bcast

_dir1 := .. . 
OBJS := ConfigHelper.os Config.os pho_bcast.os BcastPHO.os ../public.os

INCDIR += $(_dir1)

LDFLAGS += $(commonlink) $(icelink) 

include $(TOPDIR)/build/common.mk

