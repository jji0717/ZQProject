include  $(top_srcdir)/config/compileflags.mk
_files := ConsoleScanner.l ConsoleGrammar.y ../TsPathAdmin.ICE
GENFILES := ConsoleScanner.cpp ConsoleGrammar.hpp ConsoleGrammar.cpp TsPathAdmin.h TsPathAdmin.cpp


AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=SiteAdmin
SiteAdmin_SOURCES=TsPathAdmin.cpp ConsoleScanner.cpp ConsoleGrammar.cpp PathAdmin.cpp AdminConsole.cpp PathAdminMain.cpp 
SiteAdmin_LDADD=
AM_LDFLAGS=$(commonlink) $(icelink) 

ConsoleGrammar.hpp ConsoleGrammar.cpp : ConsoleGrammar.y
	@echo "compiling $< ..."
	-rm -rf $@
	bison -dvtl -oConsoleGrammar.cpp $< 
ConsoleScanner.cpp : ConsoleScanner.l
	@echo "compiling $< ..."
	-rm -rf $@
	flex -o$@ $< 
TsPathAdmin.h TsPathAdmin.cpp : ../TsPathAdmin.ICE
	@echo "compiling $< ..." 
	$(SLICE2CPP) $(SLICEFLAGS) $<
clean-local:
	-rm -rf TsPathAdmin.{h,cpp} $(bin_PROGRAMS).dbg rubbish $(csbin_dir)/$(bin_PROGRAMS)*  $(cssyb_dir)/$(bin_PROGRAMS)*
tsflag:="elf"
goal:=SiteAdmin
include $(top_srcdir)/config/endpros.mk
