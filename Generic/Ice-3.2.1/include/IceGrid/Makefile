# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

include $(top_srcdir)/config/Make.rules

install::
	@if test ! -d $(install_includedir)/IceGrid ; \
	then \
	    echo "Creating $(install_includedir)/IceGrid..." ; \
	    $(call mkdir,$(install_includedir)/IceGrid) ; \
	fi

	@for i in *.h ; \
	do \
	    echo "Installing $$i" ; \
	    $(INSTALL_DATA) $$i $(install_includedir)/IceGrid/$$i ; \
	    chmod a+r $(install_includedir)/IceGrid/$$i ; \
	done
