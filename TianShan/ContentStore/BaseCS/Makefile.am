include $(top_srcdir)/config/compileflags.mk
INCDIR:=.. ../../CPE/common ../../CDNLib/CDNSS ../../CDNLib ../../Ice @_vstrm_inc_dir@ @_ctf_inc_dir@ ../../CPE/CPH_CDN
NEEDDIS:=$(addprefix -I,$(INCDIR))
CXXFLAGS+=$(NEEDDIS)

VPATH:=:$(top_srcdir)/TianShan/CPE/common:../../CDNLib/CDNSS:../../CDNLib:../../Ice:@_vstrm_inc_dir@:@_ctf_inc_dir@:
CXXFLAGS+=-DLOGFMTWITHTID -DNAS -DDISABLE_PACING -D_LINUX -DEXCLUDE_VSTRM_API -D_VSTRM_DATA_TYPE_DEFINED -D_CTF_DATA_TYPE_DEFINED
LDFLAGS+=-L..
AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=BaseCS
BaseCS_SOURCES=BaseCSCmd.cpp cachepNSS_draft.cpp cspNTFS.cpp C2HttpClient.cpp IdxFileParserEnvironment.cpp IndexFileParser.cpp ParseIndexFile.cpp SimpleXMLParser.cpp VV2Parser.cpp VvxParser.cpp
BaseCS_LDADD=
AM_LDFLAGS=-lContentStore $(commonlink) $(icelink) -L@_ctf_lib_dir@  -lCTF -lboost_regex 
.PHONY:rubbish
lofiles:=SimpleXMLParser.lo C2HttpClient.lo IdxFileParserEnvironment.lo IndexFileParser.lo ParseIndexFile.lo VV2Parser.lo VvxParser.lo
$(lofiles:%.lo=%.o) : rubbish
rubbish : $(lofiles)
	touch rubbish
tsflag:="elf"
goal:=$(bin_PROGRAMS)
clean-local:
	-rm -rf $(bin_PROGRAMS).dbg rubbish $(csbin_dir)/$(bin_PROGRAMS)*  $(cssyb_dir)/$(bin_PROGRAMS)*
include $(top_srcdir)/config/endpros.mk
