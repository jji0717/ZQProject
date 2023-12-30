# Linux Makefile for rtfLib real time trickfile library
#

# basic path and tool definitions

ifdef RTF_EMBEDDED ############################################################
ifndef $(BSPPATH)
BSPPATH = /opt/timesys/linux/4.1/iq80331/src/2.4.21-timesys-4.1/kernel
endif
CC	= armv5l-linux-gcc
LD	= armv5l-linux-ld
CFLAGS	= -D_LINUX -D_EMBEDDED 
LDFLAGS += -s
ifdef KERNEL
CFLAGS += -D__KERNEL__ -DMODULE -DEXPORT_SYMTAB
endif
else ##########################################################################
CC	= gcc
AR	= ar
CFLAGS	= -D_LINUX
ARFLAGS	= -q
endif #########################################################################

RM      = /bin/rm -rf

ifdef DEBUG ###################################################################
CFLAGS		+= -g -D_DEBUG
OBJDIRNAME	= LINUX_DBG
else ##########################################################################
CFLAGS		+= -O3
OBJDIRNAME	= LINUX_REL
endif #########################################################################

# source directories

ROOTDIR		= $(PWD)
RTFSRCDIR	= $(ROOTDIR)
VVXSRCDIR	= $(RTFSRCDIR)/VVXLib
VV2SRCDIR	= $(RTFSRCDIR)/VV2Lib
APPSRCDIR	= $(RTFSRCDIR)/RTFTEST

# object directories

RTFOBJDIR	= $(RTFSRCDIR)/$(OBJDIRNAME)
VVXOBJDIR	= $(VVXSRCDIR)/$(OBJDIRNAME)
VV2OBJDIR	= $(VV2SRCDIR)/$(OBJDIRNAME)
APPOBJDIR	= $(APPSRCDIR)/$(OBJDIRNAME)

# library directories

RTFLIBDIR	= $(RTFOBJDIR)

# other defines

RTFMODNAME	= rtf.o
RTFLIBNAME	= libRTF.a
RTFLIBREF	= RTF
RTFAPPNAME	= rtfTest

# extended tool definitions

INCPATH		= -I$(RTFSRCDIR) -I$(VVXSRCDIR) -I$(APPSRCDIR) -I$(VV2SRCDIR)
ifdef KERNEL
INCPATH		+= -I$(BSPPATH)/include
endif
CFLAGS		+= $(INCPATH)

# source file lists

RTFSRCS		= \
		$(RTFSRCDIR)/RTFBuf.c \
		$(RTFSRCDIR)/RTFFlt.c \
		$(RTFSRCDIR)/RTFGop.c \
		$(RTFSRCDIR)/RTFLib.c \
		$(RTFSRCDIR)/RTFObj.c \
		$(RTFSRCDIR)/RTFOut.c \
		$(RTFSRCDIR)/RTFPat.c \
		$(RTFSRCDIR)/RTFPic.c \
		$(RTFSRCDIR)/RTFPes.c \
		$(RTFSRCDIR)/RTFPkt.c \
		$(RTFSRCDIR)/RTFPmt.c \
		$(RTFSRCDIR)/RTFSeq.c \
		$(RTFSRCDIR)/RTFSes.c \
		$(RTFSRCDIR)/RTFSys.c \
		$(RTFSRCDIR)/RTFWin.c \
		$(RTFSRCDIR)/RTFIdx.c \
		$(RTFSRCDIR)/RTFIdxVV2.c \
		$(RTFSRCDIR)/RTFIdxVVX.c \
		$(RTFSRCDIR)/RTFVcd.c \
		$(RTFSRCDIR)/RTFVcdMPEG2.c \
		$(RTFSRCDIR)/RTFVcdH264.c \
		$(RTFSRCDIR)/RTFVcdVC1.c \
		$(RTFSRCDIR)/RTFCas.c \
		$(RTFSRCDIR)/RTFCasVmx.c \
		$(RTFSRCDIR)/RTFCasPan.c \
		$(RTFSRCDIR)/RTFCat.c

VVXSRCS		= \
		$(VVXSRCDIR)/VVXLib.c

VV2SRCS		= \
		$(VV2SRCDIR)/VV2Lib.c

APPSRCS		= \
		$(APPSRCDIR)/rtfTest.c

# include file lists

RTFINCS		= \
		$(RTFSRCDIR)/RTFBuf.h \
		$(RTFSRCDIR)/RTFFlt.h \
		$(RTFSRCDIR)/RTFGop.h \
		$(RTFSRCDIR)/RTFLib.h \
		$(RTFSRCDIR)/RTFPrv.h \
		$(RTFSRCDIR)/RTFObj.h \
		$(RTFSRCDIR)/RTFOut.h \
		$(RTFSRCDIR)/RTFPat.h \
		$(RTFSRCDIR)/RTFPes.h \
		$(RTFSRCDIR)/RTFPic.h \
		$(RTFSRCDIR)/RTFPkt.h \
		$(RTFSRCDIR)/RTFPmt.h \
		$(RTFSRCDIR)/RTFSeq.h \
		$(RTFSRCDIR)/RTFSes.h \
		$(RTFSRCDIR)/RTFSys.h \
		$(RTFSRCDIR)/RTFWin.h \
		$(RTFSRCDIR)/RTFIdx.h \
		$(RTFSRCDIR)/RTFIdxVV2.h \
		$(RTFSRCDIR)/RTFIdxVVX.h \
		$(RTFSRCDIR)/RTFVcd.h \
		$(RTFSRCDIR)/RTFVcdPrv.h \
		$(RTFSRCDIR)/RTFVcdMPEG2.h \
		$(RTFSRCDIR)/RTFVcdH264.h \
		$(RTFSRCDIR)/RTFVcdVC1.h \
		$(RTFSRCDIR)/RTFCas.h \
		$(RTFSRCDIR)/RTFCasPrv.h \
		$(RTFSRCDIR)/RTFCasVmx.h \
		$(RTFSRCDIR)/RTFCasPan.h \
		$(RTFSRCDIR)/RTFCat.h

VVXINCS		= \
		$(VVXSRCDIR)/VVXLib.h \
		$(VVXSRCDIR)/VVXDef.h \
		$(VVXSRCDIR)/Vvx.h

VV2INCS		= \
		$(VV2SRCDIR)/VV2Lib.h

APPINCS =

# object file lists

RTFOBJS		= \
		$(RTFOBJDIR)/RTFBuf.o \
		$(RTFOBJDIR)/RTFFlt.o \
		$(RTFOBJDIR)/RTFGop.o \
		$(RTFOBJDIR)/RTFLib.o \
		$(RTFOBJDIR)/RTFObj.o \
		$(RTFOBJDIR)/RTFOut.o \
		$(RTFOBJDIR)/RTFPat.o \
		$(RTFOBJDIR)/RTFPes.o \
		$(RTFOBJDIR)/RTFPic.o \
		$(RTFOBJDIR)/RTFPkt.o \
		$(RTFOBJDIR)/RTFPmt.o \
		$(RTFOBJDIR)/RTFSeq.o \
		$(RTFOBJDIR)/RTFSes.o \
		$(RTFOBJDIR)/RTFSys.o \
		$(RTFOBJDIR)/RTFWin.o \
		$(RTFOBJDIR)/RTFIdx.o \
		$(RTFOBJDIR)/RTFIdxVV2.o \
		$(RTFOBJDIR)/RTFIdxVVX.o \
		$(RTFOBJDIR)/RTFVcd.o \
		$(RTFOBJDIR)/RTFVcdMPEG2.o \
		$(RTFOBJDIR)/RTFVcdH264.o \
		$(RTFOBJDIR)/RTFVcdVC1.o \
		$(RTFOBJDIR)/RTFCas.o \
		$(RTFOBJDIR)/RTFCasVmx.o \
		$(RTFOBJDIR)/RTFCasPan.o \
		$(RTFOBJDIR)/RTFCat.o


VVXOBJS		= \
		$(VVXOBJDIR)/VVXLib.o

VV2OBJS		= \
		$(VV2OBJDIR)/VV2Lib.o

APPOBJS		= \
		$(APPOBJDIR)/rtfTest.o

ifdef KERNEL
RTFSRCS		+= $(RTFSRCDIR)/RTFMain.c $(RTFSRCDIR)/RTFMainTest.c
RTFOBJS		+= $(RTFOBJDIR)/RTFMain.o $(RTFOBJDIR)/RTFMainTest.o
endif

# make target definitions

MODULE		= $(RTFOBJDIR)/$(RTFMODNAME)
LIB				= $(RTFOBJDIR)/$(RTFLIBNAME)
APP				= $(APPOBJDIR)/$(RTFAPPNAME)

# make targets

.PHONY:		clean

ifdef KERNEL
all:		$(MODULE)
else
all:		$(MODULE) $(LIB) $(APP)
endif

clean:
		$(RM) $(MODULE) $(APP) $(LIB) $(RTFOBJS) $(VVXOBJS) $(VV2OBJS) $(APPOBJS)
		
$(APP):		$(LIB) $(APPSRCS) $(APPINCS)
		$(CC) $(CFLAGS) $(APPSRCS) -L$(RTFLIBDIR) -l$(RTFLIBREF) -o$(APP)

$(LIB):		$(RTFOBJS) $(VVXOBJS) $(VV2OBJS)
		$(AR) $(ARFLAGS) $(LIB) $(RTFOBJS) $(VVXOBJS) $(VV2OBJS)

$(RTFOBJS):	$(RTFOBJDIR)/%.o: $(RTFSRCDIR)/%.c $(RTFINCS) $(VVXINCS) $(VV2INCS)
		$(CC) -c $(CFLAGS) $(RTFSRCDIR)/$(*F).c -o $@

$(VVXOBJS):	$(VVXOBJDIR)/%.o: $(VVXSRCDIR)/%.c $(RTFINCS) $(VVXINCS)
		$(CC) -c $(CFLAGS) $(VVXSRCDIR)/$(*F).c -o $@

$(VV2OBJS):	$(VV2OBJDIR)/%.o: $(VV2SRCDIR)/%.c $(RTFINCS) $(VV2INCS)
		$(CC) -c $(CFLAGS) $(VV2SRCDIR)/$(*F).c -o $@

$(MODULE):	$(RTFOBJS) $(VVXOBJS) $(VV2OBJS)
		-@echo Linking $(MODULE)...
		@$(LD) -o $(MODULE) $(LDFLAGS) -r $(RTFOBJS) $(VVXOBJS) $(VV2OBJS)
		-@echo Done.
