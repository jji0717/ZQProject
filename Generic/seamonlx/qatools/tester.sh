#!/bin/bash
#
# sample of cmd line execution:
#
# tester -p /full/path/exe -n nodename
# tester -p /usr/local/hemphill/bcast -n guidev
#
# expands to the following command
# gnome-terminal --title="BCAST" -x  /usr/local/hemphill/bcast guidev
#
#
# $0 = tester
# $1 = -p
# $2 = full path
# $3 = -n
# $4 = node
# Number of expected args should be 4

Number_of_expected_args=4 

E_WRONG_ARGS=85
script_parameters="-p /pathto/where/bcastapp/located -n nodename "

if [ $# -ne $Number_of_expected_args ]
then
  echo ;
  echo "Usage: `basename $0` $script_parameters";
  echo ;
  # `basename $0` is the script's filename.
  exit $E_WRONG_ARGS
fi


BCTPATH=
SERVER=
while getopts "hp:n:" options; do
  case $options in
    p ) BCTPATH=$OPTARG;;
    n ) SERVER=$OPTARG;;
    h ) echo $usage;;
    \? ) echo "Usage: `basename $0` $script_parameters";
         exit 1;;
    * ) echo "Usage: `basename $0` $script_parameters";
          exit 1;;

  esac
done

filename="$2/bcast"
prelim="gnome-terminal --title=\"BCAST\" -x"


if [ -x "$filename" ]; then
	cmd="$prelim $BCTPATH/bcast $SERVER";
	echo `$cmd`;
else
	echo "File $filename not found.";
fi;




