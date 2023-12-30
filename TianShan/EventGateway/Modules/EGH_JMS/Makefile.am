include  $(top_srcdir)/config/compileflags.mk

_files:=Message.ICE
GENFILES:=Message.h Message.cpp MessageRecord.h MessageRecord.cpp

SOURCESALL:=Message.cpp MessageRecord.cpp ConfigHelper.cpp EGH_JMS.cpp JMSEventHelper.cpp \
			JMSMessageChannel.cpp JMSMessageTransporter.cpp 
CXXFLAGS+=-I../.. -I$(top_srcdir)/Generic/JndiClient
VPATH:=:@VPATHS@:

AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libEGH_JMS.la
libEGH_JMS_la_SOURCES=$(SOURCESALL)
libEGH_JMS_la_LIBADD=
#libEGH_JMS_la_LDFLAGS=@lib_versioninfo@
AM_LDFLAGS=-lJndiClient -lZQCommon -lTianShanCommon -lTianShanIce $(icelink) -lboost_regex
.SECONDARY: $(GENFILES)
.PHONY:all clean test gent icetest

Message.h Message.cpp : Message.ICE
	@echo 'compiling ICE file $< ...... '
	$(SLICE2CPP) $(SLICEFLAGS) $<

MessageRecord.h MessageRecord.cpp : Message.ICE
	@echo 'compiling ICE freeze file $< ...... '
	$(SLICE2FREEZE) $(SLICEFLAGS) \
	--dict EventGateway::JMS::MessageRecord,long,EventGateway::JMS::Message MessageRecord $<
clean-local:
	-rm -rf {Message,MessageRecord}.{h,cpp} $(csbin_dir)/$(lib_LTLIBRARIES:%.la=%)*  $(cssyb_dir)/$(lib_LTLIBRARIES:%.la=%)*
tsflag:="so"
goal:=libEGH_JMS.so
include $(top_srcdir)/config/endpros.mk
 

