# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= .

include $(top_srcdir)/config/Make.rules

SUBDIRS		= config src include

ifneq ($(MAKECMDGOALS),install)
    SUBDIRS		:= $(SUBDIRS) test
endif

INSTALL_SUBDIRS	= $(install_bindir) $(install_libdir)$(cpp11libdirsuffix) $(install_includedir) \
	$(install_configdir) $(install_mandir)

install:: install-common
	@for subdir in $(INSTALL_SUBDIRS); \
	do \
	    if test ! -d $(DESTDIR)$$subdir ; \
	    then \
		echo "Creating $(DESTDIR)$$subdir..." ; \
		mkdir -p $(DESTDIR)$$subdir ; \
		chmod a+rx $(DESTDIR)$$subdir ; \
	    fi ; \
	done
ifeq ($(create_runpath_symlink),yes)
	@if test -h $(embedded_runpath_prefix) ; \
	then \
	     if `\rm -f $(embedded_runpath_prefix) 2>/dev/null`; \
              then echo "Removed symbolic link $(embedded_runpath_prefix)"; fi \
        fi
	@if ! test -d $(embedded_runpath_prefix) ; \
	then \
	     if `ln -s $(prefix) $(embedded_runpath_prefix) 2>/dev/null`; \
              then echo "Created symbolic link $(embedded_runpath_prefix) --> $(prefix)"; fi \
	fi
endif

$(EVERYTHING)::
	@for subdir in $(SUBDIRS); \
	do \
	    echo "making $@ in $$subdir"; \
	    ( cd $$subdir && $(MAKE) $@ ) || exit 1; \
	done

test::
	@python $(top_srcdir)/allTests.py
nsVerifier: Glacier2Lib

IcePatch2: IcePatch2Lib

IceStorm: IceBox IceStormLib IceGridLib Glacier2Lib Freeze

IceGrid: IceGridLib IceStorm IcePatch2Lib Freeze IceSSL IceBox IceXML

IceDiscovery IceLocatorDiscovery: Ice

all:: $(SUBDIRS)

$(SUBDIRS):
	@echo "making all in $@"
	@$(MAKE) all --directory=$@

$(EVERYTHING_EXCEPT_ALL)::
	@for subdir in $(SUBDIRS); \
        do \
            if test -d $$subdir ; \
            then \
                echo "making $@ in $$subdir"; \
                ( cd $$subdir && $(MAKE) $@ ) || exit 1; \
            fi; \
        done
