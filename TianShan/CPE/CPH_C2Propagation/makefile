TOPDIR := ../../..

include $(TOPDIR)/build/defines.mk

SOLIB := CPH_C2Propagation

_files := ../CPE.ICE
GENFILES := ../CPE.cpp ../CPE.h

OBJS := CPE.os BaseCPH.os BaseFilter.os BaseGraph.os C2HttpClient.os ConfigHelper.os \
	    CPH_C2Propagation.os CPH_C2PropagationCfg.os CStdFileIo.os \
	    CStdFileIoFactory.os FilesetTarget.os \
	    HTTPClientFactory.os HTTPPropagation.os HTTPPropSource.os \
	    IdxFileParserEnvironment.os IndexFileParser.os NormalizeSparseFile.os QueueBufMgr.os \
	    SimpleXMLParser.os TargetFactoryI.os VV2Parser.os VvxParser.os ParseIndexFile.os

INCDIR += .. ../common ../Pacing ../CPH_CDN ../../CDNLib/CDNSS $(_vstrm_inc_dir) $(_ctf_inc_dir)
VPATH += ..:../common:../../CDNLib/CDNSS:../CPH_CDN:  

CXXFLAGS +=  -DLOGFMTWITHTID -DDISABLE_PACING -DNAS -DEXCLUDE_VSTRM_API -D_LINUX -D_CTF_DATA_TYPE_DEFINED -D_VSTRM_DATA_TYPE_DEFINED
LDFLAGS += $(commonlink) $(icelink) -L$(_ctf_lib_dir) -lCTF

include $(TOPDIR)/build/common.mk

../CPE.h ../CPE.cpp: $(_files)
	@echo "compiling ICE file $< ..."
	$(Q)$(SLICE2CPP) $(SLICEFLAGS) --output-dir .. $<

