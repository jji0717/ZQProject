# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

!include $(top_srcdir)/config/Make.rules.mak

CA_FILES =  iceca \
            iceca.bat \
	    ImportKey.class

install::
	@for %i in ( $(CA_FILES) ) do \
	    @echo "Installing %i" && \
	    copy %i $(install_bindir)
