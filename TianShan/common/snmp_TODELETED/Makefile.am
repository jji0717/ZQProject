include  $(top_srcdir)/config/compileflags.mk

CXXFLAGS  += -fPIC
AUTOMAKE_OPTIONS=foreign 
lib_LIBRARIES=libSnmpLib.a
libSnmpLib_a_SOURCES=oid.cpp SnmpUtil.cpp SubAgent.cpp ZQSnmpMgmt.cpp conv_netsnmp.cpp
.SECONDARY: $(GENFILES)
.PHONY:all clean testp 
testp:test.cpp
	$(CXX) -g2 -O0 -o SnmpLibTest test.cpp -I$(top_srcdir)/Common -L. -L$(_bindir) -lsnmp -lSnmpLib -lZQCommon -Wl,-rpath=$(_bindir)

tsflag:="ar"
goal:=libSnmpLib.a
include $(top_srcdir)/config/endpros.mk

