include  $(top_srcdir)/config/compileflags.mk
EXTRA_FILES := CSGrammer.output
GENFILES := CSScanner.cpp CSGrammer.cpp CSGrammer.hpp
#EXTRA_FILES := CSGrammer.output
if PREBUILDFLAG
endif
CXXFLAGS+=-D__STRICT_ANSI__
VPATH=:..:@VPATHS@:
LDFLAGS+=-L..
AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=ContentClient
ContentClient_SOURCES=ContentClient.cpp CSScanner.cpp CSGrammer.cpp
ContentClient_LDADD=
#AM_CPPFLAGS=$(CXXFLAGS)
AM_LDFLAGS=-lContentStore $(commonlink) $(icelink) @_readline_dir@/lib/libreadline.a @_readline_dir@/lib/libhistory.a @_termcap_dir@/lib/libtermcap.a $(LDFLAGS) 
prebuild:
	@-rm -rf unistd.h
CSGrammer.hpp CSGrammer.cpp: CSGrammer.y
	@echo "compiling $< ..."
	@-$(RM) $(RMFLAGS) $@
	@bison -dvtl -oCSGrammer.cpp $<
CSScanner.cpp: CSScanner.l
	@echo "compiling $< ..."
	@-$(RM) $(RMFLAGS) $@
	@flex -o$@ $<
clean-local:
	-rm -rf $(bin_PROGRAMS).dbg rubbish $(csbin_dir)/$(bin_PROGRAMS)*  $(cssyb_dir)/$(bin_PROGRAMS)*
tsflag:="elf"
goal:=$(bin_PROGRAMS)
clean-local:
	-rm -rf $(bin_PROGRAMS).dbg rubbish $(csbin_dir)/$(bin_PROGRAMS)*  $(cssyb_dir)/$(bin_PROGRAMS)*
include  $(top_srcdir)/config/endpros.mk
