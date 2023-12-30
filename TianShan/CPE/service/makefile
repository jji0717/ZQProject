TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

TARGET := CPESvc

_dir1 := ..

_files := $(_dir1)/CPE.ICE
GENFILES := $(addprefix $(_dir1)/, CPE.cpp CPE.h ContentToProvision.cpp ContentToProvision.h)

OBJS := CPE.o ContentToProvision.o CPECfg.o CPCClient.o CPEFactory.o \
		CPEEnv.o CPEImpl.o QueueBufMgr.o ProvisionCmds.o \
		ErrorProcess.o ProvisionFactory.o ProvisionState.o ProvisionStore.o ProvisionResourceBook.o \
		BaseIO.o NTFSIO.o CECommon.o CmdLine.o \
		FTPAccount.o FtpSock.o FtpConnection.o IOInterface.o\
		FtpPushSess.o FtpServer.o FtpSite.o \
		FtpsXfer.o FtpXferEx.o TermService.o utils.o \
		CStdFileIoFactory.o	CStdFileIo.o \
		ZQDaemon.o ConfigHelper.o getopt.o CPESvc.o 

_dir2 :=  $(_dir1)/PT_FtpServer $(_dir1)/common  $(_dir1)/../common/snmp
INCDIR += $(_dir1) $(_dir2)

VPATH += ..:../PT_FtpServer:../common 

LDFLAGS += -lServiceShell $(snmplink) $(commonlink) $(icelink) -ldl -lpthread

include $(TOPDIR)/build/common.mk  

$(_dir1)/CPE.h $(_dir1)/CPE.cpp: $(_files)
	@echo "compiling ICE file $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) $< --output-dir $(dir $(_files))

$(_dir1)/ContentToProvision.h $(_dir1)/ContentToProvision.cpp: $(_files)
	@echo "generate ICE freeze file $< ..."
	$(Q)$(SLICE2FREEZE) $(SLICEFLAGS) \
	--index \
	"TianShanIce::ContentProvision::ContentToProvision, \
	TianShanIce::ContentProvision::ProvisionSessionEx,contentKey" \
	ContentToProvision $< --output-dir $(dir $(_files))
