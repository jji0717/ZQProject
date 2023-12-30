TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk 

SOLIB := PHO_ERM

OBJS := ConfigHelper.os definition.os Pho_ERM.os \
		PhoAllocationOwnerImpl.os \
		PhoEdgeRM.os PhoEdgeRMEnv.os PhoNSSEdgeRM.os \
		public.os S6EdgeRM.os S6Client.os

INCDIR += .. $(TOPDIR)/Common/DataPostHouse $(TOPDIR)/Common/RtspLib
VPATH += ..

LDFLAGS += $(commonlink) -lDataPostHouse

include $(TOPDIR)/build/common.mk
