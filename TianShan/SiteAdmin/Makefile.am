include  $(top_srcdir)/config/compileflags.mk

_files := SiteAdminSvc.ICE
GENFILES := SiteAdminSvc.h SiteAdminSvc.cpp SiteToMount.h SiteToMount.cpp AppToMount.h AppToMount.cpp	\
	SiteToTxn.h SiteToTxn.cpp MountToTxn.h MountToTxn.cpp TxnToEvent.h TxnToEvent.cpp
SOURCESALL:=ZQDaemon.cpp ConfigHelper.cpp SiteAdminSvc.cpp SiteToMount.cpp \
		AppToMount.cpp SiteToTxn.cpp MountToTxn.cpp TxnToEvent.cpp \
		EventSenderManager.cpp SaIceImpl.cpp \
		SaService.cpp SiteAdminDatabase.cpp SiteAdminEnv.cpp SiteAdminIceObjFactory.cpp \
		SiteAdminSvcImpl.cpp StreamEventReceiver.cpp \
		StreamEventSinker.cpp TxnTransfer.cpp TxnWatchDog.cpp

CXXFLAGS+=-I../Sentry/EventSink
CXXFLAGS+=-DWITH_ICESTORM -DCHECK_WITH_GLOG
VPATH=:@VPATHS@:
AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=SiteAdmin
SiteAdmin_SOURCES=$(SOURCESALL)
SiteAdmin_LDADD=
AM_LDFLAGS=-lServiceShell $(commonlink) $(icelink) $(snmplink)

$(GENFILES): $(_files)
	@echo "compiling ICE file $< ..."
	$(SLICE2CPP) $(SLICEFLAGS) $<
	$(SLICE2FREEZE) $(SLICEFLAGS)	\
	--index "TianShanIce::Site::SiteToMount,TianShanIce::Site::AppMount,siteName,case-insensitive" SiteToMount $<
	$(SLICE2FREEZE) $(SLICEFLAGS)	\
	--index "TianShanIce::Site::AppToMount,TianShanIce::Site::AppMount,appName,case-insensitive" AppToMount $<
	$(SLICE2FREEZE) $(SLICEFLAGS)	\
	--index "TianShanIce::Site::SiteToTxn,TianShanIce::Site::LiveTxn,siteName,case-insensitive" SiteToTxn $<
	$(SLICE2FREEZE) $(SLICEFLAGS)	\
	--index "TianShanIce::Site::MountToTxn ,TianShanIce::Site::LiveTxn,mountedPath,case-insensitive" MountToTxn $<
	$(SLICE2FREEZE) $(SLICEFLAGS)	\
	--index "TianShanIce::Site::TxnToEvent,TianShanIce::Site::TxnEvent,identTxn" TxnToEvent $<

clean-local:
	-rm -rf {AppToMount,MountToTxn,SiteAdminSvc,SiteToMount,SiteToTxn,TxnToEvent}.{h,cpp} $(bin_PROGRAMS).dbg rubbish $(csbin_dir)/$(bin_PROGRAMS)*  $(cssyb_dir)/$(bin_PROGRAMS)*
tsflag:="elf"
goal:=SiteAdmin
include $(top_srcdir)/config/endpros.mk
