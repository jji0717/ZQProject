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
	@if test ! -d $(install_includedir)/Slice ; \
	then \
	    echo "Creating $(install_includedir)/Slice..." ; \
	    $(call mkdir,$(install_includedir)/Slice) ; \
	fi

	@for i in *.h ; \
	do \
	    echo "Installing $$i" ; \
	    $(INSTALL_DATA) $$i $(install_includedir)/Slice/$$i ; \
	    chmod a+r $(install_includedir)/Slice/$$i ; \
	done
