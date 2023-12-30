TOPDIR := ../../../..

include $(TOPDIR)/build/defines.mk

SOLIB := EGH_FileWriter

OBJS := ConfigHelper.os Text.os EventDispatcher.os EventHelper.os EGH_FileWriter.os 

INCDIR += ../..

LDFLAGS := -lZQCommon $(icelink) 

include $(TOPDIR)/build/common.mk

test: rotate

rotate: rotate.cpp
	$(Q)$(CXX) -ggdb -Wall -o rotate \
	rotate.cpp EventHelper.cpp EventDispatcher.cpp $(ZQProjsPath)/Common/ConfigHelper.cpp $(ZQProjsPath)/Common/Text.cpp \
	-I$(ZQProjsPath)/Common -I$(ZQProjsPath)/TianShan/Shell/ZQSNMPManPkg -I../..  \
	-L. -L$(ZQProjsPath)/TianShan/bin64 $(icelink) -lZQCommon
