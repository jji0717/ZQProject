TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

LIB := StreamService

_files := StreamService.ICE Playlist.ICE
_filter := memoryDebug.o 

GENFILES := StreamService.h StreamService.cpp Playlist.h Playlist.cpp

OBJS := $(filter-out $(_filter), $(patsubst %.cpp, %.o, $(wildcard *.cpp))) $(patsubst %.ICE, %.o, $(_files))

CXXFLAGS += -DWITH_ICESTORM

VPATH += ../Ice

include $(TOPDIR)/build/common.mk

%.h %.cpp: %.ICE
	@echo "compiling ICE file $< ..."
	@$(SLICE2CPP) -I../Ice $(SLICEFLAGS) $<
