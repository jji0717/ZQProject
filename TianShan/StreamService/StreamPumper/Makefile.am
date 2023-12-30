include  $(top_srcdir)/config/compileflags.mk

GENFILES := StreamService.h StreamService.cpp Playlist.h Playlist.cpp
SOURCESALL:=StreamService.cpp Playlist.cpp EventSender.cpp renewTicket.cpp \
		Scheduler.cpp SsEnvironment.cpp SsServiceImpl.cpp SsStreamBase.cpp \
		SsStreamEventCenter.cpp SsStreamImpl.cpp SsStreamImplControl.cpp StreamFactory.cpp
CXXFLAGS+=-DWITH_ICESTORM

AUTOMAKE_OPTIONS=foreign
lib_LIBRARIES=libStreamPumper.a
libStreamPumper_a_SOURCES=$(SOURCESALL)
libStreamPumper_a_LIBADD=

StreamService.h StreamService.cpp: StreamService.ICE
	@echo "compiling ICE file $< ..."
	$(SLICE2CPP) $(SLICEFLAGS) $<
Playlist.h Playlist.cpp: Playlist.ICE
	@echo "compiling ICE file $< ..."
	$(SLICE2CPP) $(SLICEFLAGS) $<
clean-local:
	-rm -rf {Playlist,StreamService}.{h,cpp}
tsflag:="ar"
goal:=libStreamPumper.a
include $(top_srcdir)/config/endpros.mk

