#!/bin/sh
#pcp:
#	g++ -g -Wall -fPIC -I/usr/include/python2.6/ -lpython2.6 -lboost_python -shared -o pcp.so c2py.cpp

TOPDIR := ../../../..

include $(TOPDIR)/build/defines.mk

SOLIB := AquaClient

OBJS := CdmiFuseOps.os  c2py.os base_64.os   CURLClient.os  sha1.os cachelayer.os  CDMIHttpClient.os  hmac_sha1.os

ifeq (${_centos_ver},6)
INCDIR += /usr/include/python2.6/ $(_curl_dir)/include $(TOPDIR)/Common ./ $(_jsoncpp_dir)/include
else ifeq (${_centos_ver},7)
INCDIR += /usr/include/python2.7/ $(_curl_dir)/include $(TOPDIR)/Common ./ $(_jsoncpp_dir)/include
endif
VPATH += ../../

CXXFLAGS := -D_FILE_OFFSET_BITS=64 -D_LINUX  -DUSE_SSLEAY -DUSE_OPENSSL 

ifeq (${_centos_ver},6)
LDFLAGS := -L$(_curl_dir)/lib -lcurl -L$(_jsoncpp_dir)/libs -ljson -lZQCommon -lpython2.6 -lboost_python
else ifeq (${_centos_ver},7)
LDFLAGS := -L$(_curl_dir)/lib -lcurl -L$(_jsoncpp_dir)/libs -ljson -lZQCommon -lpython2.7 -lboost_python
endif

include $(TOPDIR)/build/common.mk

