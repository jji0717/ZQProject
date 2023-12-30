TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

TARGET	:= tslicsys

OBJS	:= LicenseFileGenerate.o getopt.o 

INCDIR += $(TOPDIR)/Common $(TOPDIR)/Generic/CdmiFuse/jsoncpp-src-0.5.0/include

LDFLAGS += -L$(_jsoncpp_dir)/libs -ljson -lZQCommon

include $(TOPDIR)/build/common.mk
