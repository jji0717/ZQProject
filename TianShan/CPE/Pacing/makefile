TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

SOLIB := Pacing

OBJS := Pacing.os PacedIndex.os PacingLogic.os VvcIndexHelper.os

INCDIR += .. ../common $(_ctf_inc_dir) $(_vstrm_inc_dir)
VPATH += ..:../common 

CXXFLAGS += -DPACING_EXPORTS -DLOGFMTWITHTID -D_LINUX 
LDFLAGS += $(commonlink) $(icelink) -L$(_ctf_lib_dir)

include $(TOPDIR)/build/common.mk

