TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

SOLIB := SSM_LiveChannel

OBJS := $(patsubst %.cpp, %.os, $(wildcard *.cpp)) ConfigHelper.os SessionContext_ice.os StreamIdx.os auth5i.os hmac_md5.os

_ice_file := SessionContext_ice.ICE

GENFILES := SessionContext_ice.h SessionContext_ice.cpp StreamIdx.h StreamIdx.cpp 

INCDIR += $(TOPDIR)/TianShan/OpenVBO $(TOPDIR)/TianShan/StreamSmith
CXXFLAGS += -DWITH_ICESTORM
LDFLAGS  += -lZQCommon -lTianShanCommon -lTianShanIce $(icelink) -lnsl -lboost_regex
VPATH += $(TOPDIR)/TianShan/OpenVBO

include $(TOPDIR)/build/common.mk

SessionContext_ice.h SessionContext_ice.cpp: $(_ice_file)
	@echo "compiling ICE source $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $<


StreamIdx.h StreamIdx.cpp: $(_ice_file)
	@echo "compiling ICE freeze $< ..."
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
		--index "LiveChannel::StreamIdx, LiveChannel::SessionContext, streamID" StreamIdx $<
