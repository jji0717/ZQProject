# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

include $(top_srcdir)/config/Make.rules

all::
	echo ""

install::
	@if test ! -d $(prefix)/config ; \
	then \
	    echo "Creating $(prefix)/config..." ; \
	    $(call mkdir,$(prefix)/config) ; \
	fi
	$(call installdata,templates.xml,$(prefix)/config)
	$(call installdata,convertssl.py,$(prefix)/config)
	$(call installdata,upgradeicegrid.py,$(prefix)/config)
	$(call installdata,upgradeicestorm.py,$(prefix)/config)
	$(call installdata,icegrid-slice.3.1.ice.gz,$(prefix)/config)

