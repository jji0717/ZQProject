TOPDIR := ../../../..

include $(TOPDIR)/build/defines.mk

TARGET := DsmccCRGClient


OBJS := ClientSession.o ConfigHelper.o DsmccCRGClient.o DsmccSocket.o \
				SingleSession.o DsmccMsg.o getopt.o

VPATH :=$(TOPDIR)/TianShan/CRG/DsmccCRG $(TOPDIR)/Common
INCDIR += $(TOPDIR)/TianShan/CRG/DsmccCRG $(TOPDIR)/TianShan/StreamSmith/LSCP/lscpLib \
			$(TOPDIR)/Common


LDFLAGS +=$(commonlink) $(icelink) -lServiceShell -lLSCP -ldl -lpthread

include $(TOPDIR)/build/common.mk  
