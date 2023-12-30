TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

TARGET := NSS

_dir1 := ../StreamPumper
_files := $(_dir1)/StreamService.ICE $(_dir1)/Playlist.ICE
GENFILES := StreamService.h StreamService.cpp Playlist.h Playlist.cpp

OBJS := ZQDaemon.o ConfigHelper.o NGODSession.o NSSConfig.o NSSEnv.o NSSService.o NSSUtil.o

INCDIR += ../StreamPumper ../../ContentStore/A3CS ../../common/snmp

CXXFLAGS += -DWITH_ICESTORM

#LDFLAGS := -Wl,--start-group -lStreamPumper -lContentStore -lNGODCS --end-group $(commonlink) $(icelink) $(snmplink) -lServiceShell
LDFLAGS := -Wl,--start-group -lStreamPumper -lContentStore -lNGODCS $(commonlink) $(icelink) $(snmplink) -lServiceShell -lpthread

include $(TOPDIR)/build/common.mk

Playlist.h Playlist.cpp: $(word 2, $(_files))
	@echo "compiling ICE file $< ..."
	@$(SLICE2CPP) -I../Ice $(SLICEFLAGS) $<

StreamService.h StreamService.cpp: $(word 1, $(_files))
	@echo "compiling ICE file $< ..."
	@$(SLICE2CPP) -I../Ice $(SLICEFLAGS) $<
