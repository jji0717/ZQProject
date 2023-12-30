TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

SOLIB := PHO_VSS

OBJS := ConfigHelper.os Configuration.os \
	pho_VSS.os PhoCisco.os PhoHsnTree.os \
	PhoNGOD.os PhoNGOD_DVBC.os phoStorageLink.os public.os

LDFLAGS += $(commonlink) $(icelink) 

include $(TOPDIR)/build/common.mk

