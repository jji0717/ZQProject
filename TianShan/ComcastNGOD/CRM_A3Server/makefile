TOPDIR := ../../..
include $(TOPDIR)/build/defines.mk
SOLIB := CRM_A3Server

_files := A3Module.ice
GENFILES := A3Module.cpp A3Module.h AssetIdx.cpp AssetIdx.h VolumeIdx.cpp VolumeIdx.h 

OBJS := A3Module.os AssetIdx.os VolumeIdx.os \
		A3Client.os A3Common.os A3ModuleImpl.os HttpClient.os \
		A3MsgHandler.os A3Environment.os A3DllMain.os \
		ConfigHelper.os 

INCDIR += $(TOPDIR)/Common/HttpEngine 

CXXFLAGS += -DWITH_ICESTORM 

LDFLAGS += -lTianShanIce -lTianShanCommon -lZQCommon  $(icelink)        

include $(TOPDIR)/build/common.mk  

%.h %.cpp: %.ice
	@echo "compiling ICE file $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $<

AssetIdx.h AssetIdx.cpp: $(_files)
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
	--index \
	"A3Module::AssetIdx,A3Module::A3Content,assetKey,case-insensitive" \
	AssetIdx $<

VolumeIdx.h VolumeIdx.cpp: $(_files)
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
	--index \
	"A3Module::VolumeIdx,A3Module::A3Content,fullVol,case-insensitive" \
	VolumeIdx $<

