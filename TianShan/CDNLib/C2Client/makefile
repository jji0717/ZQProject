TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

TARGET := C2Client

OBJS := C2Client.o ClientCommand.o CommandCenter.o ConnectionFactory.o HttpDialog.o HttpProtocol.o HttpSession.o PoormanOpt.o SessionDataRecorder.o SessionRecordAnalyzer.o 


INCDIR += $(TOPDIR)/Common/DataPostHouse \
          $(TOPDIR)/Common

LDFLAGS := -lDataPostHouse -lZQCommon -lboost_regex -lnl -lreadline -lncurses

include $(TOPDIR)/build/common.mk

