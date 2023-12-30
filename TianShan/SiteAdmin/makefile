TOPDIR := ../..

include $(TOPDIR)/build/defines.mk

TARGET := SiteAdmin

_files := SiteAdminSvc.ICE

GENFILES := SiteAdminSvc.h SiteAdminSvc.cpp SiteToMount.h SiteToMount.cpp AppToMount.h AppToMount.cpp \
            SiteToTxn.h SiteToTxn.cpp MountToTxn.h MountToTxn.cpp TxnToEvent.h TxnToEvent.cpp

OBJS := ZQDaemon.o ConfigHelper.o \
	$(patsubst %.cpp,%.o,$(filter-out %.h,$(GENFILES))) $(patsubst %.cpp,%.o,$(filter-out stdafx.cpp,$(wildcard *.cpp)))

INCDIR += ../Sentry/EventSink 

CXXFLAGS := -DWITH_ICESTORM -DCHECK_WITH_GLOG
LDFLAGS  := -lServiceShell $(commonlink) $(icelink) $(snmplink) -ldl -lpthread

include $(TOPDIR)/build/common.mk

$(GENFILES): $(_files)
	@echo $(OBJS)
	@echo "compiling ICE file $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $<
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
   --index "TianShanIce::Site::SiteToMount,TianShanIce::Site::AppMount,siteName,case-insensitive" SiteToMount $<
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
   --index "TianShanIce::Site::AppToMount,TianShanIce::Site::AppMount,appName,case-insensitive" AppToMount $<
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
   --index "TianShanIce::Site::SiteToTxn,TianShanIce::Site::LiveTxn,siteName,case-insensitive" SiteToTxn $<
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
   --index "TianShanIce::Site::MountToTxn ,TianShanIce::Site::LiveTxn,mountedPath,case-insensitive" MountToTxn $<
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
   --index "TianShanIce::Site::TxnToEvent,TianShanIce::Site::TxnEvent,identTxn" TxnToEvent $<
