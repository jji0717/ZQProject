include  $(top_srcdir)/config/compileflags.mk
CXX:=g++
CXXFLAGS+=-I../..
VPATH:=:@VPATHS@:
AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libEGH_FileWriter.la
libEGH_FileWriter_la_SOURCES=ConfigHelper.cpp Text.cpp EventDispatcher.cpp EventHelper.cpp EGH_FileWriter.cpp
libEGH_FileWriter_la_LIBADD=
#libEGH_FileWriter_la_LDFLAGS=@lib_versioninfo@
AM_LDFLAGS=-lZQCommon $(icelink)

test: rotate
rotate: rotate.cpp
	$(CXX) -ggdb -Wall -o rotate	\
	rotate.cpp EventHelper.cpp EventDispatcher.cpp $(ZQProjsPath)/Common/ConfigHelper.cpp $(ZQProjsPath)/Common/Text.cpp	\
	-I$(ZQProjsPath)/Common -I$(ZQProjsPath)/TianShan/Shell/ZQSNMPManPkg -I../..	\
	-L. -L$(ZQProjsPath)/TianShan/bin64 $(icelink) -lZQCommon
clean-local:
	-rm -rf $(csbin_dir)/$(lib_LTLIBRARIES:%.la=%)*  $(cssyb_dir)/$(lib_LTLIBRARIES:%.la=%)*
tsflag:="so"
goal:=libEGH_FileWriter.so
include $(top_srcdir)/config/endpros.mk	

