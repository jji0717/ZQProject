TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

TARGET := C2SS

_dir1 := ../StreamPumper
_dir2	:= $(TOPDIR)/TianShan/Ice

_files := $(_dir1)/StreamService.ICE $(_dir1)/Playlist.ICE $(_dir2)/TsStreamer.ICE

GENFILES := StreamService.h StreamService.cpp Playlist.h Playlist.cpp TsStreamer.h TsStreamer.cpp

OBJS := ZQDaemon.o ConfigHelper.o  C2ContentAttrCache.o C2SSCfg.o C2SSEnv.o C2SSService.o \
		IndexFileParser.o SimpleXMLParser.o IdxFileParserEnvironment.o  \
		C2Client.o RequestHandle.o LocateRequest.o GetRequest.o

INCDIR += ../StreamPumper ../../common/snmp $(TOPDIR)/Common $(TOPDIR)/Common/libasync ../../Ice $(TOPDIR)/TianShan/CDNLib/CDNSS $(TOPDIR)/TianShan/CDNLib/CDNSS/C2Streamer $(_vstrm_inc_dir)

VPATH += $(TOPDIR)/TianShan/CDNLib/CDNSS $(TOPDIR)/TianShan/CDNLib/CDNSS/C2Streamer $(TOPDIR)/TianShan/StreamSmith/NodeContentStore

CXXFLAGS += -DWITH_ICESTORM -DEXCLUDE_VSTRM_API

#LDFLAGS := -Wl,--start-group -lStreamPumper -lasync --end-group $(commonlink) $(icelink) $(snmplink) -lServiceShell
LDFLAGS := -Wl,--start-group -lStreamPumper -lasync  $(commonlink) $(icelink) $(snmplink) -lServiceShell -lpthread

include $(TOPDIR)/build/common.mk

Playlist.h Playlist.cpp: $(word 2, $(_files))
	@echo "compiling ICE file $< ..."
	@$(SLICE2CPP) -I../Ice $(SLICEFLAGS) $<

StreamService.h StreamService.cpp: $(word 1, $(_files))
	@echo "compiling ICE file $< ..."
	@$(SLICE2CPP) -I../Ice $(SLICEFLAGS) $<
	
TsStreamer.h TsStreamer.cpp: $(word 3, $(_files))
	@echo "compiling ICE file $< ..."
	@$(SLICE2CPP) -I../Ice $(SLICEFLAGS) $<
