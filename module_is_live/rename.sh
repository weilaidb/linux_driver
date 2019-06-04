#!/bin/sh
export CURPATH=$(pwd)
CHANGENAME=`basename ${CURPATH}`
echo current path is ${CURPATH}
echo change name is ${CHANGENAME}
mv *.c ${CHANGENAME}.c
sed -i 's/=[_a-z]\+.o/='$(echo $CHANGENAME)'\.o/g' Makefile
