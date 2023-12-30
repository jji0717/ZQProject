TOPDIR := ../../../..

include $(TOPDIR)/build/defines.mk

TARGET := C2Client

OBJS := main.o C2Client.o C2ClientConf.o C2ResultCollection.o getopt.o ConfigHelper.o \
		IndexFileParser.o SimpleXMLParser.o IdxFileParserEnvironment.o  \
		RequestHandle.o LocateRequest.o GetRequest.o TransferDelete.o OutputHandle.o \
		ConcurrentRequest.o LoopRequest.o

CXXFLAGS := -DEXCLUDE_VSTRM_API

VPATH += $(TOPDIR)/TianShan/CDNLib/CDNSS $(TOPDIR)/TianShan/CDNLib/CDNSS/C2Pump $(TOPDIR)/Common

INCDIR += $(TOPDIR)/TianShan/CDNLib/CDNSS $(TOPDIR)/TianShan/CDNLib/CDNSS/C2Pump $(TOPDIR)/Common $(_vstrm_inc_dir)

LDFLAGS += -lZQCommon -lasync 

include $(TOPDIR)/build/common.mk

