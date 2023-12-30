#! /bin/sh
find . -name Makefile.am -exec tar rfv ../mtree1.tar {} \;
tar xf ../mtree.tar 
tree -a -C
