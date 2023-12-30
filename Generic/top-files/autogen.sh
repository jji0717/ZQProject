#!/bin/bash
export ZQProjsPath=$PWD
find ./ -name  makefile | xargs rm -rf
aclocal --force
libtoolize -f -c 
autoheader -f
autoconf  -f 
automake -a -f -c
./configure  --enable-debug=yes --with-3rd=/opt/sdk/ --prefix=/test1
cd ./TianShan/EdgeRM/Pho_ERM;make 3rdh &> /dev/null ;cd -
make

