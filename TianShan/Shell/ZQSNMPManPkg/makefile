TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

SOLIB := SnmpManPkg
OBJS := SnmpManPkg.os SnmpSubagent.os SnmpUtil.os VarInfo.os Mib.os Var.os 

LDFLAGS += -lsnmp 

include $(TOPDIR)/build/common.mk

