TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

LIB := SnmpLib

SRC_EXT := hpp

OBJS := oid.o SnmpUtil.o SubAgent.o ZQSnmpMgmt.o conv_netsnmp.o

include $(TOPDIR)/build/common.mk

test: test.cpp
	$(CXX) -g2 -O0 -o SnmpLibTest test.cpp -I$(TOPDIR)/Common -L. -L$(_bindir) -lsnmp -lSnmpLib -lZQCommon -Wl,-rpath=$(_bindir) 
