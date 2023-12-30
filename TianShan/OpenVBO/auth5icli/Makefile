TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

TARGET := auth5i

OBJS := auth5i.o getopt.o hmac_md5.o main.o
 
VPATH += $(TOPDIR)/TianShan/OpenVBO $(TOPDIR)/Common

LDFLAGS += $(addprefix -l, ZQCommon)

include $(TOPDIR)/build/common.mk


