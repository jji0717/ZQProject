#! /bin/sh
rm -rf  aclocal.m4 config.log Makefile.in config.status config.h Makefile autom4te.cache config.h.in config.h.in~ configure libtool config.guess config.sub ltmain.sh missing depcomp COPYING stamp-h1

i=0
dirs=("Common build TianShan test")
dirslen=${#dirs[@]}
#echo "dirslen=$dirslen"

while [ $i -lt $dirslen ]
do
#	echo ${dirs[$i]}
	find ./${dirs[$i]} -iname Makefile | xargs rm -rf  
	find ./${dirs[$i]} -iname Makefile.in | xargs  rm -rf 
	let i++
done
#for dir in $dirs
#do
#	find ./$dir -iname Makafile -print
#done


