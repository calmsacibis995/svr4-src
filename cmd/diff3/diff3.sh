#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)diff3:diff3.sh	1.7"

e=
case $1 in
-*)
	e=$1
	shift;;
esac
if test $# = 3 -a \( -f $1 -o -c $1 \) -a \( -f $2 -o -c $2 \) -a \( -f $3 -o -c $3 \)
then
	:
else
	echo usage: diff3 file1 file2 file3 1>&2
	exit
fi
f1=$1 f2=$2 f3=$3
if [ -c $f1 ]
then
	cat $f1 >/tmp/d3c$$
	f1=/tmp/d3c$$
fi
if [ -c $f2 ]
then
	cat $f2 >/tmp/d3d$$
	f2=/tmp/d3d$$
fi
if [ -c $f3 ]
then
	cat $f3 >/tmp/d3e$$
	f3=/tmp/d3e$$
fi

trap "rm -f /tmp/d3[a-e]$$" 0 1 2 13 15
diff $f1 $f3 >/tmp/d3a$$
diff $f2 $f3 >/tmp/d3b$$
/usr/lib/diff3prog $e /tmp/d3[ab]$$ $f1 $f2 $f3
