#if DEBUG
#AM_CFLAGS=-g3 -O0
#AM_CXXFLAGS=-g3 -O0
#else
#AM_CFLAGS=-O2
#AM_CXXFLAGS=-O2
#endif

AUTOMAKE_OPTIONS=foreign

SUBDIRS := Common Generic  TianShan test/libtool_use
CURRENTPATH=$(PWD)



INCLUDES=

#export AM_CFLAGS AM_CXXFLAGS  INCLUDES 
ACLOCAL_AMFLAGS=-I m4
EXTRA_DIST=autogen.sh
