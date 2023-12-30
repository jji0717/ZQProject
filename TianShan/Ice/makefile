TOPDIR := ../..

include $(TOPDIR)/build/defines.mk

SOLIB := TianShanIce

_extra := $(_ice_slice_dir)/ZQIceVersion.ICE
_files := $(wildcard *.ICE) $(_extra)

GENFILES := $(patsubst %.ICE,%.h,$(notdir $(_files))) $(patsubst %.ICE,%.cpp,$(notdir $(_files))) 
OBJS := $(patsubst %.ICE,%.os,$(notdir $(_files)))

VPATH := $(_ice_slice_dir)

include $(TOPDIR)/build/common.mk

%.h %.cpp: %.ICE
	@echo "compiling ICE file $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $< --output-dir .

