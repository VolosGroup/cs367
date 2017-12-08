#!/bin/bash
mkdir -p tests

if [ $1 ]
then
	echo "Creating $1 test(s)."
	a=$1
else
	echo "Creating default # of tests: 1"
	a=1
fi

if [ $2 ]
then
	echo "Each test will have $2 address(es)."
	b=$2
else
	echo "Each test will have the default # of addresses: 10"
	b=10
fi
for ((i=1;i<100;i++));
do
	rm -f tests/testfile$i
	echo "Creating file testfile$i"
	for ((j=0;j<b;j++));
	do
		echo "$(shuf -i0-999 -n1)">> tests/testfile$i
	done
	echo "-1">>tests/testfile$i
done
