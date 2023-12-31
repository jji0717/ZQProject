# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..

!include $(top_srcdir)/config/Make.rules.mak

!if "$(CPP_COMPILER)" != "BCC2006"
SUBDIRS		= icecpp
!endif

#SUBDIRS		= $(SUBDIRS) IceUtil Slice slice2cpp slice2cs slice2vb slice2freeze slice2freezej slice2docbook slice2java \
#		  slice2py slice2cppe slice2javae slice2rb slice2html Ice IceXML IceSSL IceBox IcePatch2 Glacier2 Freeze \
#		  FreezeScript IceStorm IceGrid ca
SUBDIRS		= $(SUBDIRS) IceUtil Slice slice2cpp slice2freeze  \
		  slice2py Ice IceBox IceXML Freeze FreezeScript IceStorm
		  

$(EVERYTHING)::
	@for %i in ( $(SUBDIRS) ) do \
	    @if exist %i \
	        @echo "making $@ in %i" && \
	        cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1
