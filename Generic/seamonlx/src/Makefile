TARGET = seamonlx

# Add new cpp files here
SRCS = seamonlx.cpp \
	seamonlxalert.cpp \
	seamonlxUDEV.cpp \
	seamonlxSHAS.cpp \
	seamonlxXMLRPC.cpp \
	common.cpp \
	trace.cpp \
	HealthStateChange.cpp \
	StringResource.cpp \
	Server.cpp \
	StorageAdapters.cpp \
	ServerHwXmlrpc.cpp \
	ServerEnvXmlrpc.cpp \
	StorageAdapterXmlrpc.cpp \
	MgmtPort.cpp \
	MgmtPortXmlrpc.cpp \
	SystemHealth.cpp \
	SystemHealthXmlrpc.cpp \
	AlertXmlrpc.cpp \
	AlertId.cpp \
	AlertComponent.cpp \
	EnclProcessors.cpp \
	EnclProcessorXmlrpc.cpp \
	SvcsXmlrpc.cpp \
	RpmsXmlrpc.cpp \
	ConfigXmlrpc.cpp \
	BMCXmlrpc.cpp \
	Disks.cpp \
	DiskXmlrpc.cpp \
	SHASConfig.cpp \
	SHASConfigXmlrpc.cpp \
	SHASCounters.cpp \
	SHASCountersXmlrpc.cpp \
	NetworkAdapters.cpp \
	seamonlx_init.cpp \
	seamonlx_monitor.cpp \
	InfinibandAdapters.cpp \
	InfinibandAdaptersXmlrpc.cpp \
	NetworkAdaptersXmlrpc.cpp

OBJS = $(SRCS:.cpp=.o)

WORKSPACE_ROOT = $(shell cd ../../;pwd)
XMLRPC_C_CONFIG = xmlrpc-c-config
BCSAMPLE_ROOT = /usr/local/shas/bcsample
RCINCLUDES = $(BCSAMPLE_ROOT)/includes
XMLRPC_INCLUDES = $(shell $(XMLRPC_C_CONFIG) c++2 abyss-server --cflags) 
INCLUDES = -I$(RCINCLUDES) $(XMLRPC_INCLUDES) -I../includes
RCLIBPATH = $(BCSAMPLE_ROOT)/libs/unix/linux/x86_64
RCLIBS = -lrt -lbcapi -lrctl
IDIR = $(WORKSPACE_ROOT)/includes
LIBS = -lrt 
XMLRPC_LIBS = $(shell $(XMLRPC_C_CONFIG) c++2 abyss-server --libs)

CFLAGS += -g -W -Wall $(INCLUDES) -I$(IDIR) -I../../../Common 

.PHONY: all clean
CUR_EDIRPATH=$(shell pwd)
all: $(TARGET)

$(TARGET): $(OBJS)
	cd ../../../build&& $(MAKE) ZQCommon 
	@echo $(shell pwd)
	cp  ../../../TianShan/bin64/libZQCommon.so ./
	@echo building executable $@
	$(CXX) $(CFLAGS) $(INCLUDES) $(LIBS) $(XMLRPC_LIBS) -o $@ $^ $(RCLIBPATH)/libbcapi.a $(RCLIBPATH)/librctl.a  -L. -lZQCommon -Wl,-rpath='$$ORIGIN'
$(OBJS):%.o:%.cpp
	$(CXX) -c $(CFLAGS) $(INCLUDES) $<

depend:
	makedepend -f- -- $(SRCS) > .depend_file

clean:
	rm -f *.o *~ core libZQCommon.so $(TARGET) .depend_file

