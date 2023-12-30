include $(top_srcdir)/config/compileflags.mk

ICE_FILES:= TianShanIce.ICE  TsAppChOD.ICE TsCache.ICE  TsEdgeResource.ICE \
	   		TsRepository.ICE  TsSRM.ICE  TsStreamer.ICE  TsAppBcast.ICE  \
	    	TsApplication.ICE  TsContentProv.ICE  TsEvents.ICE  TsSite.ICE  \
		    TsStorage.ICE  TsTransport.ICE ZQIceVersion.ICE 

ICES:= $(ICE_FILES)

MID_HEAD_FILES:=$(ICE_FILES:.ICE=.h)
MID_SRC_FILES:=$(ICE_FILES:.ICE=.cpp)
MID_OBJ_FILES:=$(ICE_FILES:.ICE=.o)

_depend := $(if $(MID_OBJ_FILES),$(MID_OBJ_FILES:%.o=.%.d),,)
GENFILES:=$(MID_HEAD_FILES) $(MID_SRC_FILES)
VPATH=:@_ice_slice_dir@:
AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libTianShanIce.la
libTianShanIce_la_SOURCES=$(MID_SRC_FILES)
#libTianShanIce_la_LDFLAGS=@lib_versioninfo@
#AM_LDFLAGS=$(LDFLAGS) 
.SECONDARY: $(GENFILES)
.PHONY:all clean test gent icetest
_h_file:=TsApplication.h TsStreamer.h TsSRM.h TsEvents.h TsTransport.h TsStorage.h
3rdh : $(_h_file)
%.h %.cpp : %.ICE  
	@echo "compiling ICE file $< ......"
	${SLICE2CPP} ${SLICEFLAGS}  $< --output-dir .
clean-local:
	-rm -rf $(_h_file) $(csbin_dir)/$(lib_LTLIBRARIES:%.la=%)*  $(cssyb_dir)/$(lib_LTLIBRARIES:%.la=%)*
tsflag:="so"
goal:=libTianShanIce.so
include $(top_srcdir)/config/endpros.mk

