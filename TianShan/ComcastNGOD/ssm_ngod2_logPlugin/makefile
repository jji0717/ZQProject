TOPDIR := ../../../

include $(TOPDIR)/build/defines.mk

SOLIB := NGODHist

OBJS :=  NGOD2mdbViewer.os ConfigHelper.os ssm_ngod2_logPlugin.os ODBCHelper.os

INCDIR += $(TOPDIR)/Common $(TOPDIR)/TianShan/Sentry/Module/MDB_web $(TOPDIR)/TianShan/Sentry/EventSink

LDFLAGS += -lZQCommon -lodbc -lmyodbc5

VPATH += $(TOPDIR)/TianShan/Sentry/Module/MDB_web 

include $(TOPDIR)/build/common.mk
