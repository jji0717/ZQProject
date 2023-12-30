TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

TARGET := LogParser

_files := LogPosition.ICE

GENFILES := LogPosition.cpp LogPosition.h

OBJS := AckWindow.o ConfigHelper.o getopt.o LiteralFunc.o LogMessageHandler.o LogMonitor.o\
        LogParserApp.o LogParserManager.o LogPosition.o LogPositionI.o TimeConv.o \
		MessageSenderPump.o MessageSource.o RuleHandler.o SenderManager.o
	 

 



INCDIR += .

LDFLAGS += $(commonlink) $(icelink) -lServiceShell -lboost_regex -ldl -lpthread -lboost_system-mt

include $(TOPDIR)/build/common.mk

LogPosition.h LogPosition.cpp: $(_files)
	@echo "generating ICE source $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $<
