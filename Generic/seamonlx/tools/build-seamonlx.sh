#!/bin/bash

echo "Building the seamonlx RPM...."
TOPDIR=`pwd`
#HOSTNAME=`hostname`
HOSTNAME=build
DATETIME=`date +%m%d%y%H%M%S`

# where we get the executable on the development machine
SEAMONLX_SRC_DIR=${TOPDIR}/../src

# where to install the executables on target machine
TARGET_DIR=usr/local/seamonlx/bin

# package version TBD
VERSION=0.0
RELEASE=1     
BUILD_NUMBER=${HOSTNAME}_${DATETIME}

ARCH=`uname -m`

RPM=seamonlx-$VERSION-$RELEASE.$ARCH.rpm

# BUILD DIRECTORIES
[ -d $TOPDIR/BUILD ] || mkdir -p $TOPDIR/BUILD
[ -d $TOPDIR/STAGING ] || mkdir -p $TOPDIR/STAGING

# where to put the final rpm
[ -d $TOPDIR/RPMS/$ARCH/ ] || mkdir -p -m 0777  $TOPDIR/RPMS/$ARCH/


rpmbuild -v -bb seamonlx.spec --buildroot=${TOPDIR}/STAGING \
                                  --define "seamonlxDir ${SEAMONLX_SRC_DIR}" \
                                  --define "targetdir ${TARGET_DIR}" \
                                  --define "sversion $VERSION" \
                                  --define "srelease $RELEASE" \
                                  --define "sbuildnumber $BUILD_NUMBER" \
                                  --define "_topdir $TOPDIR"

rm -rf $TOPDIR/BUILD
rm -rf $TOPDIR/STAGING
