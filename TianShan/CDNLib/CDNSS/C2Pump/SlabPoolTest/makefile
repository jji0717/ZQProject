TOPDIR := ../../../../..

include $(TOPDIR)/build/defines.mk

TARGET := slabPoolTest

OBJS := main.o test.o SlabPool.o SlabPoolDefine.o

VPATH += $(TOPDIR)/TianShan/CDNLib/CDNSS/C2Pump

INCDIR += $(TOPDIR)/TianShan/CDNLib/CDNSS/C2Pump $(TOPDIR)/Common $(gtest)

LINKDIR += $(gtest)

LDFLAGS += -lZQCommon -lgtest.a -lboost_thread-mt

include $(TOPDIR)/build/common.mk
