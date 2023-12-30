TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

SOLIB := CRM_A3

INCDIR += ../../../Common/HttpEngine ../../CPE ../../common/vrep  $(_curl_dir)/include $(TOPDIR)/Common $(_jsoncpp_dir)/include \
	  ../../ContentStore/MediaClusterCS ../../ContentStore/ ../../../Generic/CdmiFuse
VPATH += .. ../../ContentStore/
OBJS := $(filter-out stdafx.os, $(patsubst %.cpp, %.os, $(wildcard *.cpp))) ConfigHelper.os HttpClient.os CPCImpl.os D4Update.os

CXXFLAGS := -DWITH_ICESTORM -D_LINUX -DUSE_SSLEAY -DUSE_OPENSSL -DLOGFMTWITHTID

LDFLAGS := -lContentStore -lvrep $(commonlink) $(icelink) -L$(_curl_dir)/lib -lcurl -L$(_jsoncpp_dir)/libs -ljson  -lAquaClient -lboost_thread-mt -lboost_system-mt

include $(TOPDIR)/build/common.mk
