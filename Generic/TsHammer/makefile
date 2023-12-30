TOPDIR := ../..

include $(TOPDIR)/build/defines.mk

TARGET := TsHammer

OBJS := AssetPool.o ScriptParser.o Hammer.o HammerSession.o HammerMonitor.o main.o

LDFLAGS += -lZQCommon -lboost_regex -lcurses

include $(TOPDIR)/build/common.mk

# vim: ts=4 sw=4 bg=dark nu
