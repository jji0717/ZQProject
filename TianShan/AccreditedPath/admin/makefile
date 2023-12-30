TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

ifeq (${_centos_ver},7)
     CXXFLAGS += -DADMIN
endif

TARGET := PathAdmin

EXTRA_FILES := ConsoleGrammar.output

_files := ConsoleScanner.l ConsoleGrammar.y ../TsPathAdmin.ICE
GENFILES := ConsoleScanner.cpp ConsoleGrammar.hpp ConsoleGrammar.cpp TsPathAdmin.h TsPathAdmin.cpp

OBJS := ConsoleScanner.o ConsoleGrammar.o TsPathAdmin.o PathAdmin.o AdminConsole.o PathAdminMain.o

LDFLAGS += $(commonlink) $(icelink)

include $(TOPDIR)/build/common.mk

ConsoleGrammar.hpp ConsoleGrammar.cpp: ConsoleGrammar.y
	@echo "compiling $< ..."
	$(Q)-$(RM) $(RMFLAGS) $@
	$(Q)bison -dvtl -oConsoleGrammar.cpp $< 

ConsoleScanner.cpp: ConsoleScanner.l
	@echo "compiling $< ..."
	$(Q)-$(RM) $(RMFLAGS) $@
	$(Q)flex -o$@ $<

TsPathAdmin.h TsPathAdmin.cpp: ../TsPathAdmin.ICE
	$(Q)echo "compiling $<"
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $<
