#!/bin/sh
# $Id: python-search.sh 12011 2011-03-13 03:09:55Z mthuurne $
for name in python python2 python2.5 python2.6 python2.7 python2.8 python2.9
do
	$name -c 'import sys; sys.exit(not((2, 5) <= sys.version_info < (3, )))' \
		2> /dev/null
	if test $? -eq 0
	then
		echo $name
		break
	fi
done
