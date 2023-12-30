TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

TARGET := StreamClient

OBJS := SSAdmin.o StreamSmithAdmin.o SSScanner.o SSGrammer.o 
_files := ../Service/StreamSmithAdmin.ICE SSScanner.l SSGrammer.y
GENFILES := StreamSmithAdmin.h StreamSmithAdmin.cpp SSScanner.cpp SSGrammer.cpp SSGrammer.hpp

LDFLAGS += $(commonlink) $(icelink) $(_readline_dir)/lib/libreadline.a $(_readline_dir)/lib/libhistory.a $(_termcap_dir)/lib/libtermcap.a

include $(TOPDIR)/build/common.mk

prebuild:
	$(Q)-$(RM) $(RMFLAGS) unistd.h

SSGrammer.hpp SSGrammer.cpp: SSGrammer.y
	@echo "compiling $< ..."
	$(Q)-$(RM) $(RMFLAGS) $@
	$(Q)bison -dvtl -oSSGrammer.cpp $< 

SSScanner.cpp: SSScanner.l
	@echo "compiling $< ..."
	$(Q)-$(RM) $(RMFLAGS) $@
	$(Q)flex -o$@ $<

StreamSmithAdmin.h StreamSmithAdmin.cpp: ../Service/StreamSmithAdmin.ICE
	@echo "compiling ICE file $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $< 
