PWD=`pwd`
WORKSPACE_ROOT=$(shell cd ..;pwd)
SRCDIR=$(PWD)/src
BUILDDIR=$(PWD)/tools
RPMDIR=$(BUILDDIR)/RPMS
KITDIR=$(WORKSPACE_ROOT)/kits
BUILD_SCRIPT="build-seamonlx.sh"
XMLRPC_VERSION="1.20.0"
XMLRPC_C=xmlrpc-c-$(XMLRPC_VERSION)
XMLRPC_TARBALL=$(PWD)/rpms/$(XMLRPC_C).gz
XMLRPC_RPM=$(PWD)/rpms/$(XMLRPC_C)-00.x86_64.rpm
XMLRPC_CONFIG=/usr/local/bin/xmlrpc-c-config
XMLRPC_INCLUDE=/usr/local/include/xmlrpc-c
XMLRPC_INSTALLED_VERSION=`/usr/local/bin/xmlrpc-c-config --version`
XMLRPC_BUILD_DIR=/tmp/$(XMLRPC_C)
SHAS_RPM_DIR=$(WORKSPACE_ROOT)/thirdparty/seachange/shas
SHAS_FILE_BASE_NAME = shas-api
SHAS_RPM_VER=$(SHAS_FILE_BASE_NAME)-2.0.1-550
#SHAS_RPM_VER=shas-api-2.0.1-550
SHAS_RPM_FILE=$(SHAS_RPM_VER).x86_64.rpm
SHAS_RPM_INSTALLED=$(shell rpm -q shas-api)

.PHONY: all clean package

all:
	@echo Building SeaMon LX libraries...
	@if [ -x $(XMLRPC_CONFIG) ] && [ $(XMLRPC_INSTALLED_VERSION) = $(XMLRPC_VERSION) ] && [ -d $(XMLRPC_INCLUDE) ] ; then \
		echo $(XMLRPC_C) ${XMLRPC_RPM} libraries are already installed. ; \
		echo XMLRPC_RPM = ${XMLRPC_RPM} ; \
	else \
		echo Installing the $(XMLRPC_C) libraries... ; \
		rpm -ivh ${XMLRPC_RPM} ; \
	fi
		
	@if [ "$(SHAS_RPM_VER)" != "$(SHAS_RPM_INSTALLED)" ] ; then \
		echo "$(SHAS_RPM_VER) NOT FOUND.  Installing $(SHAS_RPM_DIR)/$(SHAS_RPM_FILE)" ; \
		rpm -e $(SHAS_FILE_BASE_NAME) ; \
		rpm -ihv $(SHAS_RPM_DIR)/$(SHAS_RPM_FILE) ; \
	fi

	@echo Building SeaMon LX binary...
	make clean -C $(SRCDIR)
	make -C $(SRCDIR)

clean:
	@echo cleaning SeaMon LX build...
	make clean -C $(SRCDIR)
	rm -rf $(RPMDIR)/*

package:
	@echo Building SeaMon LX RPM package...
	@cd $(KITDIR) ; \
	# the following hardcoded version makes it clear this is a personal build, not from the nightly build script \
	export MGMT_MAJOR_VER="99" ; \
	export MGMT_MINOR_VER="0" ; \
	export MGMT_BUILD_VER="0" ; \
	export MGMT_ECO_VER="0" ; \
	export MGMT_BUILD_NUMBER="0" ; \
	if [ -e build-seamonlx-rpm ] ; then \
		chmod +x build-seamonlx-rpm ; \
		echo "Creating RPM for SeaMon LX" ; \
		./build-seamonlx-rpm ; \
	else \
		echo "build-seamonlx-rpm did not exist, SeaMon LX not packaged" ; \
	fi
