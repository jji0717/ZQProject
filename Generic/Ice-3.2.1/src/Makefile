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

SUBDIRS		= IceUtil \
		  icecpp \
		  Slice \
		  slice2cpp \
		  slice2cs \
		  slice2vb \
		  slice2freeze \
		  slice2freezej \
		  slice2docbook \
		  slice2java \
		  slice2py \
		  slice2cppe \
		  slice2javae \
		  slice2rb \
		  slice2html \
		  Ice \
		  IceXML \
		  IceSSL \
		  Freeze \
		  FreezeScript \
		  IceBox \
		  IceStorm \
		  Glacier2 \
		  IcePatch2 \
		  IceGrid \
		  ca

$(EVERYTHING)::
	@for subdir in $(SUBDIRS); \
	do \
	    if test -d $$subdir ; \
	    then \
		echo "making $@ in $$subdir"; \
		( cd $$subdir && $(MAKE) $@ ) || exit 1; \
	    fi; \
	done
