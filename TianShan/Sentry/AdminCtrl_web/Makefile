TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

siteAdminSvc_files := $(TOPDIR)/TianShan/SiteAdminSvc/SiteAdminSvc.ICE
tsPathAdmin_files := $(TOPDIR)/TianShan/AccreditedPath/TsPathAdmin.ICE

SOLIB := AdminCtrl_web

OBJS := AdminCtrl_web.os AdminCtrlUtil.os PathController.os SiteAdminSvc.os SiteController.os TsPathAdmin.os
				
INCDIR += $(TOPDIR)/TianShan/SiteAdminSvc $(TOPDIR)/TianShan/common $(TOPDIR)/TianShan/Ice $(TOPDIR)/Common $(ICE_ROOT)/include $(ICE_ROOT)/include/stlport $(TOPDIR)/TianShan/AccreditedPath

VPATH += .:..:../../common:$(TOPDIR)/Common:$(TOPDIR)/TianShan/Ice

LDFLAGS += $(icelink) $(commonlink)

include $(TOPDIR)/build/common.mk

$./SiteAdminSvc.h ./SiteAdminSvc.cpp: $(siteAdminSvc_files)
	@echo "compiling ICE file $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $< --output-dir .
$./TsPathAdmin.h ./TsPathAdmin.cpp: $(tsPathAdmin_files)
	@echo "compiling ICE file $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $< --output-dir .