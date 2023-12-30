include  $(top_srcdir)/config/compileflags.mk

_files := ../StreamSmith/Service/StreamSmithAdmin.ICE
GENFILES := StreamSmithAdmin.h StreamSmithAdmin.cpp

AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=DummySS
DummySS_SOURCES=StreamSmithAdmin.cpp DummySS.cpp DummyStreamSmith.cpp
DummySS_LDADD=
AM_LDFLAGS=$(commonlink) $(icelink)

StreamSmithAdmin.h StreamSmithAdmin.cpp: $(_files)
	$(SLICE2CPP) $(SLICEFLAGS) $<
clean-local:
	-rm -rf StreamSmithAdmin.{h,cpp} $(bin_PROGRAMS).dbg rubbish $(csbin_dir)/$(bin_PROGRAMS)*  $(cssyb_dir)/$(bin_PROGRAMS)*
tsflag:="elf"
goal:=DummySS
include  $(top_srcdir)/config/endpros.mk
