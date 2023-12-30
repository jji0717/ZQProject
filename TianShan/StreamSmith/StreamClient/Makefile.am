include $(top_srcdir)/config/compileflags.mk

_files := ../Service/StreamSmithAdmin.ICE SSScanner.l SSGrammer.y
GENFILES := StreamSmithAdmin.h StreamSmithAdmin.cpp SSScanner.cpp SSGrammer.cpp SSGrammer.hpp
AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=StreamClient
StreamClient_SOURCES=StreamSmithAdmin.cpp SSGrammer.cpp SSScanner.cpp SSAdmin.cpp
StreamClient_LDADD=
AM_LDFLAGS=$(commonlink) $(icelink)

prebuild:
	-rm -rf unistd.h
SSGrammer.hpp SSGrammer.cpp: SSGrammer.y
	@echo "compiling $< ...... "
	-rm -rf $@
	bison -dvtl -oSSGrammer.cpp $< 
SSScanner.cpp: SSScanner.l
	@echo "compiling $< ...... "
	-rm -rf $@
	flex -o$@ $<
StreamSmithAdmin.h StreamSmithAdmin.cpp: ../Service/StreamSmithAdmin.ICE
	@echo "compiling ICE file $< ...... "
	$(SLICE2CPP) $(SLICEFLAGS) $< 
clean-local:
	-rm -rf StreamSmithAdmin.{h,cpp} $(bin_PROGRAMS).dbg rubbish $(csbin_dir)/$(bin_PROGRAMS)*  $(cssyb_dir)/$(bin_PROGRAMS)*
tsflag:="elf"
goal:=$(bin_PROGRAMS)
include $(top_srcdir)/config/endpros.mk
