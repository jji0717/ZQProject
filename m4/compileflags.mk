CFLAGS=
CXXFLAGS=
if DEBUG
AM_CFLAGS = -g3 -O0
AM_CXXFLAGS = -g3 -O0
else
AM_CFLAGS = -g -O2
AM_CXXFLAGS =-g  -O2
CXXFLAGS +=-pipe -fomit-frame-pointer
endif
CXXFLAGS += -Wall -Wno-unknown-pragmas -DLOGFMTWITHTID $(addprefix -I, @INCDIRS@)

#LDFLAGS  += $(addprefix -L, @LINKDIRS@) -Wl,-rpath='$$ORIGIN:../bin'
LDFLAGS  += $(addprefix -L, @LINKDIRS@) 


#commonlink:= $(addprefix @_bindir@/, libTianShanCommon.la  libTianShanIce.la  libZQCommon.la  libSnmpManPkg.la)
commonlink:= $(addprefix -l, TianShanCommon TianShanIce ZQCommon)
icelink   := $(addprefix -l, IceStorm Freeze Ice IceUtil db)
snmplink  := $(addprefix -l, SnmpManPkg snmp)
#_exec_ice_bin=@_sys_interp@  --library-path @_ice_lib_dir@
#SLICE2CPP=${_exec_ice_bin} @_ice_dir@/bin/slice2cpp
SLICE2CPP=@_ice_dir@/bin/slice2cpp
SLICE2FREEZE=${_exec_ice_bin} @_ice_dir@/bin/slice2freeze
SLICEFLAGS=${addprefix -I, . @_ice_slice_dir@  @_tsdir@/Ice @_tsdir@/common}
$(warning "SLICEFLAGS99=[$(SLICEFLAGS)]")
JAVAC=@_java_dir@/bin/javac
JAVAH=@_java_dir@/bin/javah
JAR=@_java_dir@/bin/jar

csbin_dir:=$(top_srcdir)/TianShan/bin64
cslib_dir:=$(top_srcdir)/TianShan/lib64
cssyb_dir:=$(top_srcdir)/symbols
.PHONY: all clean testP test version prebuild dbgfilea preinstall lofiles
.SECONDARY:$(GENFILES)
default: all

#all: version prebuild target

