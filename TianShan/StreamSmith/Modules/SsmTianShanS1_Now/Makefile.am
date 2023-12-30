include  $(top_srcdir)/config/compileflags.mk

_ice_file := SessionContext_ice.ICE
GENFILES := SessionContext_ice.h SessionContext_ice.cpp StreamIdx.h StreamIdx.cpp

CXXFLAGS += -DWITH_ICESTORM
SOURCESALL:=ConnectService.cpp  DescribeRequest.cpp  Environment.cpp  Factory.cpp \
	 GetParamRequest.cpp  OptionRequest.cpp  PauseRequest.cpp  PingHandler.cpp  \
	 PlaylistEvent.cpp  PlayRequest.cpp  PluginEntry.cpp  RequestHandler.cpp \
	 SessionContext_ice.cpp  SessionContextImpl.cpp  SessionView.cpp  SetupRequest.cpp  \
	 StreamEvent.cpp  StreamIdx.cpp  stroprt.cpp  TeardownRequest.cpp  TsConfig.cpp \
	 WatchDog.cpp ConfigHelper.cpp SessionContext_ice.cpp StreamIdx.cpp 
AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libSSM_TianShan_S1.la
libSSM_TianShan_S1_la_SOURCES=
libSSM_TianShan_S1_la_LIBADD=
#libSSM_TianShan_S1_la_LDFLAGS=@lib_versioninfo@
AM_LDFLAGS=-lZQCommon $(tscommonlink) -lTianShanIce $(icelink)

SessionContext_ice.h SessionContext_ice.cpp: $(_ice_file)
	echo "compiling ICE source $< ...... "
	$(SLICE2CPP) $(SLICEFLAGS) $<
StreamIdx.h StreamIdx.cpp: $(_ice_file)
	echo "compiling ICE freeze $< ...... "
	$(SLICE2FREEZE) $(SLICEFLAGS) \
	--index "TianShanS1::StreamIdx, TianShanS1::SessionContext, streamID" StreamIdx $<
clean-local:
	-rm -rf $(csbin_dir)/$(lib_LTLIBRARIES:%.la=%)*  $(cssyb_dir)/$(lib_LTLIBRARIES:%.la=%)*
tsflag:="so"
goal:=libSSM_TianShan_S1.so
include $(top_srcdir)/config/endpros.mk



