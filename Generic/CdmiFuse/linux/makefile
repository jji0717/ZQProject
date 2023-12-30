TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

TARGET := cdmifuse


#OBJS := CdmiFuseLinux.o CdmiFuseOps.o CURLClient.o CDMIHttpClient.o hmac_sha1.o sha1.o base_64.o cachelayer.o FuseMirror.o
OBJS := CdmiFuseLinux.o FuseMirror.o

INCDIR += $(_curl_dir)/include $(TOPDIR)/Common ./ $(_jsoncpp_dir)/include

VPATH += ../

CXXFLAGS := -D_FILE_OFFSET_BITS=64 -D_LINUX  -DUSE_SSLEAY -DUSE_OPENSSL

LDFLAGS := -lfuse -L$(_curl_dir)/lib -lcurl -L$(_jsoncpp_dir)/libs -ljson -lZQCommon  -lAquaClient -lboost_thread-mt -lboost_system-mt

include $(TOPDIR)/build/common.mk

