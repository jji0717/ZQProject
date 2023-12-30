include  $(top_srcdir)/config/compileflags.mk


AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=LogPage
LogPage_SOURCES=LogPageHelper.cpp
LogPage_LDADD=
AM_LDFLAGS=
clean-local:
	-rm -rf $(bin_PROGRAMS).dbg $(csbin_dir)/$(bin_PROGRAMS)*  $(cssyb_dir)/$(bin_PROGRAMS)*
tsflag:="elf"
goal:=LogPage
include  $(top_srcdir)/config/endpros.mk
