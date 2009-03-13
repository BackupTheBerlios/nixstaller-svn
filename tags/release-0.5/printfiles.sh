#!/bin/sh
for F in `find $*`
do
	echo \'$F\',
done | sort

