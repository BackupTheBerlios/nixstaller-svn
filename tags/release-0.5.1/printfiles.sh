#!/bin/sh
for F in `find $*`
do
	echo $F | grep -v svn >/dev/null && [ ! -d $F ] && echo \'$F\',
done | sort

