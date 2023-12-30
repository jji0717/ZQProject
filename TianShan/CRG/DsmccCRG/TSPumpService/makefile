TOPDIR := ../../../../

include $(TOPDIR)/build/defines.mk

TARGET := TSPump

OBJS := TSPump.o TsPumperService.o ConfigHelper.o ZQDaemon.o

INCDIR += $(TOPDIR)/Common $(TOPDIR)/TianShan/Include $(TOPDIR)/TianShan/Shell/ZQCfgPkg
LDFLAGS += -lZQCommon $(snmplink) -lServiceShell -lpthread

include $(TOPDIR)/build/common.mk

