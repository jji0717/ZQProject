include defines.mk

CMEV2          := $(_cmdir)/CMEV2


_mkdep = $(if $(2), $(join $(foreach f, $(1), $(f):), $(2)), $(foreach f, $(1), $($f)))

_mod_rules := CMEV2
              			  
_lib_rules := 
 
_modules := $(call _mkdep, $(_mod_rules))
_library := $(call _mkdep, $(_lib_rules))

_everything := all install clean veryclean  

goal ?= install

default: install

sinclude .depend

.PHONY: $(_everything) $(_modules) $(_library) $(_mod_rules) $(_lib_rules)

.depend: makefile
	@echo "generating dependency file $@ ..."
	$(Q)rm -f $@; \
	echo $(call _mkdep, $(_mod_rules), $(_modules)) | sed 's/ /\n/g' >> $@; \
	echo $(call _mkdep, $(_lib_rules), $(_library)) | sed 's/ /\n/g' >> $@;

$(_everything): 
	@if [ "$@" = "all" -o "$@" = "install" ]; then \
		$(MAKE) $(_modules) || exit 1; \
	else \
		$(MAKE) $(_modules) goal=$@; \
	fi	

$(_modules) $(_library):
	$(Q)$(MAKE) -C $@ $(goal)

help::
	@echo 'Targets:'
	@echo '    clean     - remove object and executables'
	@echo '    veryclean - remove generated source files and dependency files'
	@echo '    install   - build and install all modules (default)'
	@echo 
	@echo 'make verbose =0|1 [target] 0 ==> quiet (default), 1 ==> verbose'
	@echo 'make debug   =0|1 [target] 0 ==> release, 1 ==> debug (default)'
	@echo 'make dbgfile =0|1 generate a seperate symbol file or not'
	@echo 'make <module name> goal= all|install|clean|veryclean'
	@echo 'make <targets>'
	@echo 


# dependencies
#

#$(TianShanCommon):  $(ZQCommon) $(TianShanIce)
#$(SystemShell):     $(TianShanCommon) $(SNMPManPkg) 
#$(ContentStore):    $(TianShanCommon)
#(ContentClient):   $(ZQCommon) $(ContentStore)
#(CDNCS):           $(vrep) $(ContentStore) $(SNMPManPkg) $(ServiceShell)

# vim: ts=4 sw=4 noexpandtab
