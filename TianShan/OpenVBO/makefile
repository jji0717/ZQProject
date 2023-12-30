TOPDIR := ../../
include $(TOPDIR)/build/defines.mk
TARGET := Auth5I
OBJS := hmac_md5.o auth_md5.o  auth5i.o
libAuth5I.a:$(OBJS)
	ar -r libAuth5I.a $(OBJS)
INCDIR += $(TOPDIR)/Common
LDFLAGS += -lZQCommon
include $(TOPDIR)/build/common.mk