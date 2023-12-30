TOPDIR := ../../../..

include $(TOPDIR)/build/defines.mk 

SOLIB := CRM_DSMCC

_dir1 := ..

_files := $(_dir1)/clientrequest.ice

GENFILES := $(addprefix $(_dir1)/, clientrequest.cpp clientrequest.h)

OBJS := clientrequest.os ConfigHelper.os crm_dsmcc.os \
		DoNothingRequest.os dsmccPluginEntry.os EventSink.os \
		LscPauseRequest.os LscPlayRequest.os LscStatusRequest.os \
		SetupRequest.os StreamEventSinker.os TeardownRequest.os 

INCDIR += .. . $(TOPDIR)/TianShan/StreamSmith/LSCP/lscpLib
VPATH += .. .

LDFLAGS += $(commonlink)

include $(TOPDIR)/build/common.mk

$(_dir1))/clientrequest.h $(_dir1)/clientrequest.cpp: $(_files)
	@echo "compiling ICE file $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $< --output-dir $(dir $(_files))