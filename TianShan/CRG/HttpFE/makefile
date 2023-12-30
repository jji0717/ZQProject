TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

TARGET := HttpFE

OBJS := ZQDaemon.o ConfigHelper.o HttpFEConfig.o HttpFE.o CRGateway.o

INCDIR += $(TOPDIR)/Common $(TOPDIR)/Common/HttpEngine $(TOPDIR)/Common/eloop/Http/LibHttp $(TOPDIR)/Common/eloop $(_uv_dir)/include

CXXFLAGS +=  -DCONFIG_NO_SNMP

LINKDIR += $(_uv_dir)/lib64 $(TOPDIR)/Common/eloop $(TOPDIR)/Common/eloop/Http/LibHttp
LDFLAGS +=  $(snmplink) $(commonlink) $(icelink) -luv -leloop -lHttp -lServiceShell -lboost_regex -ldl -lpthread -lboost_system-mt

include $(TOPDIR)/build/common.mk  

