include  $(top_srcdir)/config/compileflags.mk

_file:=../../CRM_C2Locator/C2Locator.ice
GENFILES:=C2Locator.h C2Locator.cpp

CXXFLAGS+=-I$(top_srcdir)/TianShan/Sentry  -I../..
VPATH=:@VPATHS@:

AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libc2loc_web.la
libc2loc_web_la_SOURCES=C2Locator.cpp c2loc_web.cpp
libc2loc_web_la_LIBADD=
#libc2loc_web_la_LDFLAGS=@lib_versioninfo@
AM_LDFLAGS=-lZQCommon -lTianShanCommon -lTianShanIce $(icelink)

C2Locator.h C2Locator.cpp : $(_file)
	@echo "compiling Ice file  $< ......"
	$(SLICE2CPP) $(SLICEFLAGS) $< --output-dir .
clean-local:
	-rm -rf C2Locator.{h,cpp} $(csbin_dir)/$(lib_LTLIBRARIES:%.la=%)*  $(cssyb_dir)/$(lib_LTLIBRARIES:%.la=%)*
tsflag:="so"
goal:=libc2loc_web.so
include $(top_srcdir)/config/endpros.mk
