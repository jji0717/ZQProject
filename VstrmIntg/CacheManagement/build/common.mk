# build vars
#

_dest    ?= $(_bindir)
_src_ext ?= cpp
_obj_ext ?= o

GEN := $(CXX) -o

osver := $(shell cat /etc/redhat-release | awk '{print $$3}'| cut -b 1 )
centos_ver := CENTOS_$(osver)

ifeq ($(debug),0)
	CXXFLAGS += -O2 -pipe -fomit-frame-pointer 
else
	CXXFLAGS += -ggdb 
endif

CXXFLAGS += -Wall -Wno-unknown-pragmas -D_LINUX_ -DLOGFMTWITHTID -D$(centos_ver)  $(addprefix -I, $(INCDIR)) 

LDFLAGS  += $(addprefix -L, $(LINKDIR)) -Wl,-rpath='$$ORIGIN:../bin'

_myname :=
_cmd :=

ifdef TARGET 
    _link   := $(strip $(TARGET))
    _target := $(_link).$(version)
    _myname := $(TARGET)
    
    _cmd := LD
endif

ifdef SOLIB
    CXXFLAGS += -fPIC
    LDFLAGS  += -shared 

    _obj_ext := os
    _link   := lib$(strip $(SOLIB)).so
    _target := $(_link).$(version)

    GEN := $(CXX) -Wl,-soname=$(_link) -o

    _myname := $(SOLIB)
    _cmd := LD
endif

ifdef LIB
    CXXFLAGS += -fPIC
    LDFLAGS :=

    _dest := $(_libdir)
    _target := lib$(strip $(LIB)).a

    GEN := $(AR) -rcs #$(ARFLAGS)  

    _myname := $(LIB)
    _cmd := AR
endif

ifndef _target
$(error 'TARGET' not defined)
endif

_depend := $(if $(OBJS),$(OBJS:%.$(_obj_ext)=.%.d),,)

# rules
#

.PHONY: all install clean veryclean version prebuild preinstall 

.SECONDARY: $(GENFILES)

default: all

ifdef MAKECMDGOALS
    ifeq ($(findstring clean,$(MAKECMDGOALS)),)
        -include $(_depend)
    endif
else
    -include $(_depend)
endif

ifdef LIB
all: version prebuild $(_target)
else
ifeq ($(debug),0)
all: version prebuild $(_target) 
else
ifeq ($(dbgfile),0)
all: version prebuild $(_target) 
else
all: version prebuild $(_target).$(_sym_ext) 
endif
endif
endif

$(_target): $(OBJS)
	@printf "%-18s %-04s %s\n" "[$(_myname)]" $(_cmd) $@
	$(Q)$(GEN) $@ $^ $(LDFLAGS) 
ifeq ($(debug),0)
	$(Q)strip -d --strip-unneeded $(_target)
endif

$(_target).$(_sym_ext): $(_target)
	$(Q)objcopy --only-keep-debug $< $@
	$(Q)objcopy --strip-debug $<
	$(Q)-objcopy --add-gnu-debuglink=$@ $<

version:
	$(Q)echo "#define VERSION \"${version}\"" > ${_prefix}/build/version.h; 

.%.d: %.$(_src_ext) $(GENFILES)
	@printf "%-18s %-04s %s\n" "[$(_myname)]" "DEP" "$@"
	$(Q)set -e; \
    $(RM) $(RMFLAGS) $@; \
	$(CXX) -MM $(addprefix -I, $(INCDIR)) $< > $@; \
	sed -i 's/\($*\)\.o[ :]*/\1.$(_obj_ext) $@ : /g' $@;

%.$(_obj_ext): %.$(_src_ext) 
	@printf "%-18s %-04s %s\n" "[$(_myname)]" "CXX" "$<" 
	$(Q)$(CXX) -c $< $(CXXFLAGS) -o $@ 

install: all preinstall
	@printf "%-18s %-04s %s\n" "[$(_myname)]" "INST" "$(_dest)/$(_target)"
	$(Q)install -D $(_target) $(_dest)/$(_target) 
ifndef LIB
	$(Q)ln -s -f $(_target) $(_dest)/$(_link)
ifeq ($(debug),1)
ifeq ($(dbgfile),1)
	$(Q)install -D $(_target).$(_sym_ext) $(_sym_dir)/$(_target).$(_sym_ext)
endif
endif
endif

clean:
	$(Q)-$(RM) -v $(OBJS) $(_target) $(_dest)/$(_target)
ifndef LIB
	$(Q)-$(RM) -v $(_dest)/$(_link) $(_target).$(_sym_ext) $(_sym_dir)/$(_target).$(_sym_ext)
endif

veryclean: clean
	$(Q)-$(RM) -v $(GENFILES) $(EXTRA_FILES) $(_depend)


# vim: ts=4 sw=4 bg=dark
