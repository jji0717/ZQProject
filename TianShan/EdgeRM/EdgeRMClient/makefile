TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

TARGET := EdgeRMClient

INCDIR += . ..

OBJS := DataTypes.o ConfigHelper.o EdgeRMClient.o ERMGrammer.o ERMScanner.o getopt.o EdgeRM.o 

LDFLAGS += $(commonlink) $(icelink)

include $(TOPDIR)/build/common.mk
	
prebuild:
	if [ -e unistd.h ]; then $(RM) $(RMFLAGS) unistd.h; fi 

unistd.h :

./EdgeRM.h ./EdgeRM.cpp: ../EdgeRM.ICE
	@echo "compiling ICE file $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $<