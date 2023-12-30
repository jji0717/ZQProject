include  $(top_srcdir)/config/compileflags.mk

_files := StreamService.ICE Playlist.ICE
_filter := memoryDebug.cpp

GENFILES := StreamService.h StreamService.cpp Playlist.h Playlist.cpp

SOURCESALL:=EventSender.cpp Playlist.cpp renewTicket.cpp Scheduler.cpp SsEnvironment.cpp \
	    SsServiceImpl.cpp SsStreamBase.cpp SsStreamEventCenter.cpp SsStreamImplControl.cpp \
	    SsStreamImpl.cpp StreamFactory.cpp StreamService.cpp  StreamService.cpp  Playlist.cpp

CXXFLAGS += -DWITH_ICESTORM
VPATH=:../Ice:@VPATHS@:
AUTOMAKE_OPTIONS=foreign
lib_LIBRARIES=libStreamService.a
libStreamService_a_SOURCES=$(SOURCESALL)
libStreamService_a_LIBADD=

lofiles:=Scheduler.lo
$(lofiles:%.lo=%.o) : rubbish
rubbish :$(lofiles)
	touch rubbish
AM_LDFLAGS=
_h_files:=Playlist.h StreamService.h
3rdh : $(_h_files)
%.h %.cpp: %.ICE
	echo "compiling ICE file $< ...... "
	$(SLICE2CPP) -I../Ice $(SLICEFLAGS) $<
clean-local:
	-rm -rf {Playlist,StreamService}.{h,cpp}	
tsflag:="ar"
goal:=libStreamService.a
include  $(top_srcdir)/config/endpros.mk

