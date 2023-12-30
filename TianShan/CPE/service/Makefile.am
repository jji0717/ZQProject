include  $(top_srcdir)/config/compileflags.mk
_dir1:=..
_files:=$(_dir1)/CPE.ICE
GENFILES:=$(addprefix $(_dir1)/, CPE.cpp CPE.h ContentToProvision.cpp ContentToProvision.h)
_dir2:=$(_dir1)/PT_FtpServer $(_dir1)/common

CXXFLAGS+=-I.. -I../PT_FtpServer  -I../common  -I../../common/snmp
 
VPATH=..:../PT_FtpServer:../common:@VPATHS@:

SOURCES:=CPE.cpp ContentToProvision.cpp CPECfg.cpp CPCClient.cpp CPEFactory.cpp	\
	CPEEnv.cpp CPEImpl.cpp QueueBufMgr.cpp ProvisionCmds.cpp ErrorProcess.cpp \
	ProvisionFactory.cpp ProvisionState.cpp ProvisionStore.cpp ProvisionResourceBook.cpp \
	BaseIO.cpp NTFSIO.cpp CECommon.cpp CmdLine.cpp FTPAccount.cpp FtpSock.cpp \
	FtpConnection.cpp IOInterface.cpp FtpPushSess.cpp FtpServer.cpp FtpSite.cpp \
	FtpsXfer.cpp FtpXferEx.cpp TermService.cpp utils.cpp CStdFileIoFactory.cpp	CStdFileIo.cpp 	\
	ZQDaemon.cpp ConfigHelper.cpp getopt.cpp CPESvc.cpp

AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=CPESvc CPEClient 
CPESvc_SOURCES=$(SOURCES)
CPESvc_LDADD= 
CPESvc_LDFLAGS=-lServiceShell $(snmplink) $(commonlink) $(icelink) $(LDFLAGS) -lSnmpLib


CPEClient_SOURCES=CPE.cpp ContentToProvision.cpp CECommon.cpp CmdLine.cpp CPEClient.cpp
CPEClient_LDADD=
CPEClient_LDFLAGS=$(commonlink) $(icelink) $(LDFLAGS) 


.SECONDARY: $(GENFILES)
.PHONY:all clean test gent icetest

$(_dir1)/CPE.h $(_dir1)/CPE.cpp: $(_files)
	@echo "compiling ICE file $< ......."
	@$(SLICE2CPP) $(SLICEFLAGS) $< --output-dir $(dir $(_files))
$(_dir1)/ContentToProvision.h $(_dir1)/ContentToProvision.cpp: $(_files)
	@echo "generate ICE freeze file $< ..."
	$(SLICE2FREEZE) $(SLICEFLAGS) \
	--index \
	"TianShanIce::ContentProvision::ContentToProvision, \
	TianShanIce::ContentProvision::ProvisionSessionEx,contentKey" \
	ContentToProvision $< --output-dir $(dir $(_files))

tsflag:="elf"
goal:=$(bin_PROGRAMS)
devarl:=$(word 1,$(bin_PROGRAMS)),$(word 2,$(bin_PROGRAMS))
clean-local:
	-rm -rf {$(devarl)}.dbg rubbish $(csbin_dir)/{$(devarl)}*  $(cssyb_dir)/{$(devarl)}*
include $(top_srcdir)/config/endpros.mk


