include  $(top_srcdir)/config/compileflags.mk
CXXFLAGS+=-I.. -I$(top_srcdir)/Common/DataPostHouse -I$(top_srcdir)/Common/RtspLib -I$(top_srcdir)/Common/RtspLib/RtspClientLib
VPATH=:..:@VPATHS@:

AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libPHO_ERM.la
libPHO_ERM_la_SOURCES=PhoAllocation.cpp SessionOfPhoAllocation.cpp ConfigHelper.cpp definition.cpp Pho_ERM.cpp \
			PhoAllocationImpl.cpp PhoAllocationOwnerImpl.cpp PhoEdgeRM.cpp PhoEdgeRMEnv.cpp \
			 PhoEdgeRMFactory.cpp PhoNSSEdgeRM.cpp public.cpp S6EdgeRM.cpp S6Client.cpp 
AM_LDFLAGS=$(commonlink) -lRtspClient -lDataPostHouse
_h_files:=PhoAllocation.h
3rdh : $(_h_files)

PhoAllocation.h PhoAllocation.cpp : PhoAllocation.ICE 
	$(SLICE2CPP) $(SLICEFLAGS) $<
	$(SLICE2FREEZE) $(SLICEFLAGS) --index "TianShanIce::EdgeResource::SessionOfPhoAllocation,::TianShanIce::EdgeResource::PhoAllocation,sessKey" SessionOfPhoAllocation $<
clean-local:
	-rm -rf {PhoAllocation,SessionOfPhoAllocation}.{h,cpp} $(csbin_dir)/$(lib_LTLIBRARIES:%.la=%)*  $(cssyb_dir)/$(lib_LTLIBRARIES:%.la=%)*
tsflag:="so"
goal:=libPHO_ERM.so
include $(top_srcdir)/config/endpros.mk

