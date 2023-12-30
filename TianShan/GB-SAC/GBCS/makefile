
TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

TARGET := GBCS

_dir1 := $(TOPDIR)/TianShan/StreamService/StreamPumper
_files := $(_dir1)/StreamService.ICE $(_dir1)/Playlist.ICE
GENFILES := StreamService.h StreamService.cpp Playlist.h Playlist.cpp

OBJS := ZQDaemon.o ConfigHelper.o cspGB.o GBCSa4Cmd.o GBCSa5Cmd.o HttpClient.o GBCSEnv.o GBCSportal.o GBCSSimpleHttpd.o GBCSReq.o GBCSService.o GBCSSimpleHttpd.o

INCDIR += $(_dir1) ../../common/snmp  ../../GB-SAC/GBCS

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
