TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

TARGET := ContentClient

OBJS := ContentClient.o CSScanner.o CSGrammer.o 

# _files := CSScanner.l CSGrammer.y
# GENFILES := CSScanner.cpp CSGrammer.cpp CSGrammer.hpp 

# EXTRA_FILES := CSGrammer.output

LDFLAGS += -lContentStore $(commonlink) $(icelink) \
		   $(_readline_dir)/lib/libreadline.a $(_readline_dir)/lib/libhistory.a $(_termcap_dir)/lib/libtermcap.a

include $(TOPDIR)/build/common.mk

prebuild:
	if [ -e unistd.h ]; then $(RM) $(RMFLAGS) unistd.h; fi 

unistd.h :
#	echo hi

# CSGrammer.hpp CSGrammer.cpp: CSGrammer.y
#	@echo "compiling $< ..."
#	$(Q)-$(RM) $(RMFLAGS) $@
#	$(Q)bison -dvtl -oCSGrammer.cpp $< 

# CSScanner.cpp: CSScanner.l
#	@echo "compiling $< ..."
#	$(Q)-$(RM) $(RMFLAGS) $@
#	$(Q)flex -o$@ $<


