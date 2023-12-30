TOPDIR := ../../../

include $(TOPDIR)/build/defines.mk

SOLIB := WebLayout

OBJS := ConsoleCommand.os DataSourceLoader.os GridRequest.os LayoutCtx.os \
	    LogRequest.os SnmpRequest.os WebLayout.os 

CXXFLAGS += -DNEWLOGFMT
LDFLAGS += -lZQCommon 

include $(TOPDIR)/build/common.mk


