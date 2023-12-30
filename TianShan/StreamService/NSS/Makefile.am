include  $(top_srcdir)/config/compileflags.mk

_dir1 := ../StreamPumper
_files := $(_dir1)/StreamService.ICE $(_dir1)/Playlist.ICE
GENFILES := StreamService.h StreamService.cpp Playlist.h Playlist.cpp
CXXFLAGS+=-I../StreamPumper  -I../../ContentStore/A3CS
CXXFLAGS += -DWITH_ICESTORM
VPATH=:@VPATHS@:

AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=NSS
NSS_SOURCES=ZQDaemon.cpp ConfigHelper.cpp NGODSession.cpp NSSConfig.cpp NSSEnv.cpp \
		NSSService.cpp NSSUtil.cpp 
NSS_LDADD=
AM_LDFLAGS=-Wl,--start-group -lStreamPumper -lContentStore -lNGODCS --end-group $(commonlink) $(icelink) $(snmplink) -lServiceShell -lSnmpLib

Playlist.h Playlist.cpp: $(word 2, $(_files))
	@echo "compiling ICE file $< ..."
	$(SLICE2CPP) -I../Ice $(SLICEFLAGS) $<
StreamService.h StreamService.cpp: $(word 1, $(_files))
	@echo "compiling ICE file $< ..."
	$(SLICE2CPP) -I../Ice $(SLICEFLAGS) $<
clean-local:
	-rm -rf $(bin_PROGRAMS).dbg rubbish $(csbin_dir)/$(bin_PROGRAMS)*  $(cssyb_dir)/$(bin_PROGRAMS)*
tsflag:="elf"
goal:=NSS
include  $(top_srcdir)/config/endpros.mk
