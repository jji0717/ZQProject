#!/bin/sh
#pcp:
#	g++ -g -Wall -fPIC -I/usr/include/python2.6/ -lpython2.6 -lboost_python -shared -o pcp.so c2py.cpp

TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

SOLIB := AquaClient

OBJS := CdmiFuseOps.os  base_64.os  crc.os  CURLClient.os  sha1.os cachelayer.os  CDMIHttpClient.os  hmac_sha1.os AquaClient_j.os AquaClient.os

ifneq (${_centos_ver},5)
    OBJS += c2py.os
endif

#INCDIR += /usr/include/python/ $(_curl_dir)/include $(TOPDIR)/Common ./ $(_jsoncpp_dir)/include
ifeq (${_centos_ver},6)
    INCDIR += /usr/include/python2.6/ $(_curl_dir)/include $(TOPDIR)/Common ./ $(_jsoncpp_dir)/include
else ifeq (${_centos_ver},7)
    INCDIR += /usr/include/python2.7/ $(_curl_dir)/include $(TOPDIR)/Common ./ $(_jsoncpp_dir)/include
endif
VPATH += ../:./python:./java

CXXFLAGS := -D_FILE_OFFSET_BITS=64 -D_LINUX  -DUSE_SSLEAY -DUSE_OPENSSL  -DLOGFMTWITHTID 

LDFLAGS :=-L$(_curl_dir)/lib -lcurl -L$(_jsoncpp_dir)/libs -ljson -lZQCommon 

#ifneq (${_centos_ver},5)
#    LDFLAGS += -lpython2.6 -lboost_python
#endif

ifeq (${_centos_ver},6)
    LDFLAGS += -lpython2.6 -lboost_python
else ifeq (${_centos_ver},7)
    LDFLAGS += -lpython2.7 -lboost_python
endif

AquaClient_j.os:NestedClient.h


NestedClient.h:java/ant.xml
	ant -f java/ant.xml
	javah -classpath java/bin/AquaClient.jar -o NestedClient.h com.xormedia.aqua.sdk.NestedClient

include $(TOPDIR)/build/common.mk
