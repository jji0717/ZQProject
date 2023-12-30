TOPDIR := ../../..
include $(TOPDIR)/build/defines.mk
SOLIB := CPH_SimpleCopy

FILES := ../CPE.ICE
GENFILES := ../CPE.cpp ../CPE.h

OBJS := BaseCPH.os CPE.os CPH_SimpleCopy.os

#incdir := .. 
INCDIR := $(INCDIR)
INCDIR += .. 
VPATH := ..: 

include $(TOPDIR)/build/common.mk


LDFLAGS += $(commonlink) $(icelink)

$(GENFILES): $(FILES)
	@echo "compiling $(FILES) ..."
	@$(ICECPP) $(SLICEFLAGS) --output-dir .. $(FILES)

