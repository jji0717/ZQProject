TOPDIR := ../

include $(TOPDIR)/build/defines.mk

TARGET := Parse

OBJS := main.o IdxFileParserEnvironment.o IndexFileParser.o SimpleXMLParser.o 

INCDIR += $(TOPDIR)/TianShan/StreamService/ServiceLib \
            $(_vstrm_inc_dir) $(_ctf_inc_dir) \
          $(TOPDIR)/TianShan/common/ \
	       $(TOPDIR)/TianShan/CDNLib 

CXXFLAGS := -DNAS -DDISABLE_PACING -D_LINUX -DEXCLUDE_VSTRM_API -DCDNCS_SERVICE -D_VSTRM_DATA_TYPE_DEFINED -D_CTF_DATA_TYPE_DEFINE
LDFLAGS := -L $(TOPDIR)/Common  -lZQCommon 

VPATH+=$(TOPDIR)/TianShan/CDNLib
include $(TOPDIR)/build/common.mk

# $(TOPDIR)/Common/DataPostHouse \
