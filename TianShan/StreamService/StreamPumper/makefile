TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

LIB := StreamPumper

GENFILES := StreamService.h StreamService.cpp Playlist.h Playlist.cpp

OBJS := StreamService.o Playlist.o EventSender.o renewTicket.o Scheduler.o \
	SsEnvironment.o SsServiceImpl.o SsStreamBase.o SsStreamEventCenter.o \
        SsStreamImpl.o SsStreamImplControl.o StreamFactory.o

CXXFLAGS += -DWITH_ICESTORM

include $(TOPDIR)/build/common.mk

StreamService.h StreamService.cpp: StreamService.ICE
	@echo "compiling ICE file $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $<

Playlist.h Playlist.cpp: Playlist.ICE
	@echo "compiling ICE file $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $<
