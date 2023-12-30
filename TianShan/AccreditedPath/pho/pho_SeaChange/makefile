TOPDIR := ../../../..

include $(TOPDIR)/build/defines.mk

SOLIB := PHO_SeaChange

_dir1 := ..
OBJS := ConfigHelper.os $(_dir1)/Config.os $(_dir1)/IpEdgePHO.os $(_dir1)/public.os $(_dir1)/Raid5sqrPHO.os pho_SeaChange.os 

INCDIR += $(_dir1)

LDFLAGS += $(commonlink) $(icelink) 

include $(TOPDIR)/build/common.mk

