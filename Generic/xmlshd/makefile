TOPDIR := ../..

include $(TOPDIR)/build/defines.mk

TARGET := xmlshd

OBJS := ConsoleCommand.o SimpleXMLParser.o XmlRPCService.o ConfigHelper.o ZQDaemon.o

INCDIR += $(TOPDIR)/Common/HttpEngine $(TOPDIR)/Common/DataPostHouse 

LDFLAGS += $(snmplink) -lZQCommon -lServiceShell -lHttpEngine -lDataPostHouse -lboost_regex -lsnmp -lcrypto -lpthread

include $(TOPDIR)/build/common.mk  

