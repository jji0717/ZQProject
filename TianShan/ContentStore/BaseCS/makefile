TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

TARGET := BaseCS
OBJS := BaseCSCmd.o cspNTFS.o C2HttpClient.o \
	cachepNSS_draft.o IdxFileParserEnvironment.o IndexFileParser.o \
	ParseIndexFile.o SimpleXMLParser.o VV2Parser.o VvxParser.o

INCDIR += ..  ../../CPE/common ../../CDNLib/CDNSS  ../../CDNLib ../../Ice \
	   $(_vstrm_inc_dir) $(_ctf_inc_dir) ../../CPE/CPH_CDN
VPATH +=$(TOPDIR)/TianShan/CPE/common ../../CDNLib/CDNSS ../../CDNLib ../../Ice $(_vstrm_inc_dir) $(_ctf_inc_dir)

CXXFLAGS += -DLOGFMTWITHTID -DNAS -DDISABLE_PACING -D_LINUX -DEXCLUDE_VSTRM_API -D_VSTRM_DATA_TYPE_DEFINED -D_CTF_DATA_TYPE_DEFINED

include $(TOPDIR)/build/common.mk

LDFLAGS += -lContentStore $(commonlink) $(icelink)  -L$(_ctf_lib_dir) -lCTF -lboost_regex


