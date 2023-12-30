TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

TARGET	:= tslicsign

OBJS	:= LicenseGenerate.o FingerPrint.o rsaPrivateKey.o getopt.o

INCDIR += $(TOPDIR)/Common /opt/sdk/3rdPartyKits/openssl/include $(TOPDIR)/Generic/CdmiFuse/jsoncpp-src-0.5.0/include

LDFLAGS += -lZQCommon -L$(_jsoncpp_dir)/libs -ljson -L$(/opt/sdk/3rdPartyKits/openssl/lib64) -lssl -lcrypto

include $(TOPDIR)/build/common.mk
