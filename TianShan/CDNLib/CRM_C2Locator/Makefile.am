include  $(top_srcdir)/config/compileflags.mk

_files:=C2Locator.ice LAMFacade.ice
GENFILES := C2Locator.h C2Locator.cpp SessionIdx.h SessionIdx.cpp LAMFacade.h LAMFacade.cpp
SOUECES_ALL:=C2Locator.cpp SessionIdx.cpp LAMFacade.cpp C2Env.cpp C2Factory.cpp C2LocatorConf.cpp C2LocatorImpl.cpp \
	      ClientManager.cpp CRM_C2Locator.cpp LocateRequestHandler.cpp ../SimpleXMLParser.cpp TransferSessionImpl.cpp \
	     TransferDeleteRequestHandler.cpp TransferPortManager.cpp ConfigHelper.cpp Text.cpp \
		 HttpClient.cpp CacheServerRequestHandle.cpp

INCDIR_CURREQUIRED=.. $(top_srcdir)/Common/HttpEngine $(top_srcdir)/Common
CXXFLAGS+=$(addprefix -I,$(INCDIR_CURREQUIRED))
CXXFLAGS += -DWITH_ICESTORM
$(warning "romalin88 CXXFLAGS=[$(CXXFLAGS)] ...VPATH=[$(VPATH)]")
VPATH=@VPATHS@


AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libCRM_C2Locator.la
libCRM_C2Locator_la_SOURCES=$(SOUECES_ALL)
libCRM_C2Locator_la_LIBADD=
#libCRM_C2Locator_la_LDFLAGS=@lib_versioninfo@ 
AM_LDFLAGS=-lTianShanIce -lTianShanCommon -lZQCommon $(icelink) -lboost_regex -lsnmp -lSnmpLib $(LDFLAGS)

.SECONDARY: $(GENFILES)
.PHONY:all clean test gent icetest

HttpClient.lo Text.lo ConfigHelper.lo:rubbish

rubbish:HttpClient.o 
	size $^
	touch rubbish


%.h %.cpp: %.ice
	@echo "compiling ICE file $< ......."
	@$(SLICE2CPP) $(SLICEFLAGS) $<
SessionIdx.h SessionIdx.cpp: C2Locator.ice
	@echo "compiling ICE freeze $< ......."
	@$(SLICE2FREEZE) $(SLICEFLAGS) --index \
			"TianShanIce::SCS::SessionIdx,TianShanIce::SCS::TransferSession,sessKey,case-sensitive" \
			SessionIdx $<

#all:
#	install -D  $(lib_LTLIBRARIES) ${top_srcdir}/TianShan/bin64/
#	cp -R .libs/libCRM_C2Locator.so* ${top_srcdir}/TianShan/bin64/.libs/
#	-rm -rf rubbish
clean-local:
	-rm -rf {C2Locator,LAMFacade,SessionIdx}.{h,cpp} $(csbin_dir)/$(lib_LTLIBRARIES:%.la=%)*  $(cssyb_dir)/$(lib_LTLIBRARIES:%.la=%)*
tsflag:="so"
goal:=libCRM_C2Locator.so
include $(top_srcdir)/config/endpros.mk

