include  $(top_srcdir)/config/compileflags.mk

dir1s:=../../MetaLib
dir2s:=../../ContentLib
icefile1s:=$(dir1s)/MetaLib.ICE
icefile2s:=$(dir2s)/ContentReplicaEx.ICE

genfile1s:=$(dir1s)/MetaLib.cpp $(dir1s)/MetaLib.h
genfile2s:=$(dir2s)/ContentReplicaEx.cpp $(dir2s)/ContentReplicaEx.h

GENFILES:=$(genfile1s) $(genfile2s)

CXXFLAGS+=-I$(dir1s) -I$(dir2s)
VPATH=:$(dir1s):$(dir2s):@VPATHS@:
SOURCESA:=ContentReplicaEx.cpp MetaLib.cpp BasePage.cpp ClibMain.cpp ClibMan_web.cpp ContentDetail.cpp \
	ShowContent.cpp ShowVolume.cpp stroprt.cpp

AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libClibMan_web.la
libClibMan_web_la_SOURCES=$(SOURCESA)
libClibMan_web_la_LIBADD=
#libClibMan_web_la_LDFLAGS=@lib_versioninfo@
AM_LDFLAGS=-lZQCommon -lTianShanCommon -lTianShanIce $(icelink)
.SECONDARY: $(GENFILES)
_h_file:=$(dir1s)/MetaLib.h
3rdh : $(_h_file)
$(genfile2s) : $(icefile2s)
	@echo  "generating ICE source $< ......"
	$(SLICE2CPP) $(SLICEFLAGS) -I$(dir1s) --output-dir $(dir2s) $< 

$(genfile1s) : $(icefile1s)
	@echo  "generating ICE source $< ......"
	$(SLICE2CPP) $(SLICEFLAGS) $< --output-dir $(dir1s) $<
clean-local:
	-rm -rf $(_h_file) $(csbin_dir)/$(lib_LTLIBRARIES:%.la=%)*  $(cssyb_dir)/$(lib_LTLIBRARIES:%.la=%)*
tsflag:="so"
goal:=libClibMan_web.so
include $(top_srcdir)/config/endpros.mk
