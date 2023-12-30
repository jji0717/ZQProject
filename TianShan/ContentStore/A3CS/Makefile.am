include  $(top_srcdir)/config/compileflags.mk

CXXFLAGS+=-I$(top_srcdir)/Common  -I../../StreamService/NSS/
VPATH:=:$(top_srcdir)/Common:@VPATHS@:

AUTOMAKE_OPTIONS=foreign
lib_LIBRARIES=libNGODCS.a
libNGODCS_a_SOURCES=HttpClient.cpp  A3HttpReq.cpp cspNGOD.cpp NGODCSEnv.cpp NGODStorePortal.cpp SimpleHttpd.cpp 
.PHONY:rubbish
lofiles:=HttpClient.lo
$(lofiles:%.lo=%.o) : rubbish 
rubbish :$(lofiles)
	touch rubbish
tsflag:="ar"
goal:=$(lib_LIBRARIES)
include $(top_srcdir)/config/endpros.mk
