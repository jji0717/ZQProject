include  $(top_srcdir)/config/compileflags.mk

_ice_files = LAMFacade.ice ModSvcIce.ice ote.ice Surf_Tianshan.ice
_dir1 := ../ICE/
_files := $(addprefix $(_dir1), $(_ice_files))
GENFILES := $(patsubst %.ice, %.h, $(_ice_files)) $(patsubst %.ice, %.cpp, $(_ice_files))	\
	Stream2Purchase.h Stream2Purchase.cpp ModSvcIce.h ModSvcIce.cpp

SLICEFLAGS += -I$(_dir1)
VPATH=:@VPATHS@:
AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=MODApp
MODApp_SOURCES=LAMFacade.cpp ModSvcIce.cpp ote.cpp Surf_Tianshan.cpp \
		Stream2Purchase.cpp ZQDaemon.cpp ConfigHelper.cpp \
		MODHelperMgr.cpp ModService.cpp ModSvcIceImpl.cpp WatchDog.cpp
MODApp_LDADD=
AM_LDFLAGS=-lServiceShell $(commonlink) $(icelink) $(snmplink) -lboost_regex

LAMFacade.h LAMFacade.cpp: $(word 1, $(_files))
	$(SLICE2CPP) $(SLICEFLAGS) $< 
ModSvcIce.h ModSvcIce.cpp Stream2Purchase.h Stream2Purchase.cpp: $(word 2, $(_files)) Surf_Tianshan.h
	$(SLICE2CPP) $(SLICEFLAGS) $<
	$(SLICE2FREEZE) $(SLICEFLAGS)	\
	--index "ZQTianShan::Application::MOD::Stream2Purchase,ZQTianShan::Application::MOD::ModPurchase,streamId" Stream2Purchase $<
ote.h ote.cpp: $(word 3, $(_files))
	$(SLICE2CPP) $(SLICEFLAGS) $<
Surf_Tianshan.h Surf_Tianshan.cpp: $(word 4, $(_files))
	$(SLICE2CPP) $(SLICEFLAGS) $<
clean-local:
	-rm -rf {LAMFacade,ModSvcIce,ote,Stream2Purchase,Surf_Tianshan}.{h,cpp} $(bin_PROGRAMS).dbg rubbish $(csbin_dir)/$(bin_PROGRAMS)*  $(cssyb_dir)/$(bin_PROGRAMS)*
tsflag:="elf"
goal:=MODApp
include  $(top_srcdir)/config/endpros.mk
