TOPDIR := ../../../..

include $(TOPDIR)/build/defines.mk

SOLIB := SSM_Richurl

OBJS := ConfigHelper.os Environment.os RequestHandler.os ssm_richurl.os urlstr.os

INCDIR += . ../.. ../../.. $(TOPDIR)/common

include $(TOPDIR)/build/common.mk

LDFLAGS += -L$(TOPDIR)/TianShan/bin -lZQCommon