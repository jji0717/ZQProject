include  $(top_srcdir)/config/compileflags.mk

SUBDIRS:=snmp vrep
CXXFLAGS  += -fPIC
ICE_FILES:=TianShanUtils.ICE  ZqSentryIce.ICE

GENFILES:=TianShanUtils.cpp TianShanUtils.h  ZqSentryIce.cpp  ZqSentryIce.h
ALL_HEERS:=$(ICE_FILES:.ICE=.h) EventChannel.h TianShanDefines.h Sentinel.h HtmlTempl.h \
		IceLog.h TianShanIceHelper.h
SOURCES:=$(ICE_FILES:.ICE=.cpp)  Sentinel.cpp ZqAdapter.cpp \
		HtmlTempl.cpp IceLog.cpp TimerWatchdog.cpp TianShanIceHelper.cpp EventChannel.cpp TianShanDefines.cpp 
AUTOMAKE_OPTIONS=foreign 
lib_LIBRARIES=libTianShanCommon.a
libTianShanCommon_a_SOURCES=$(SOURCES)
#libTianShanCommon_a_CPPFLAGS=$(CXXFLAGS)
#include_HEADERS=$(ALL_HEERS)
#AM_CPPFLAGS=$(CXXFLAGS) 
#AM_LDFLAGS=$(LDFlAGS)
.SECONDARY: $(GENFILES)
.PHONY:all clean testp 

%.h %.cpp : %.ICE
	@echo  "compling ICE  file $< ......" 
	${SLICE2CPP}  ${SLICEFLAGS}  -I../Ice  $< --output-dir . 
clean-local:
	-rm -rf {TianShanUtils,ZqSentryIce}.{h,cpp}
tsflag:="ar"
goal:=libTianShanCommon.a
include $(top_srcdir)/config/endpros.mk

