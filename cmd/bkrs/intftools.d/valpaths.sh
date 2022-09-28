#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)bkrs:intftools.d/valpaths.sh	1.2.3.1"
#script to validate a list of path names (comma- and blank-separated) using
#valpath.
# Called with three arguments: a list of options to valpath, the list of paths,
# and, optionally, an indication of the maximum number of paths allowed.
# (No max given implies there is none.)
# For example, valpaths -wgt "file1,/usr/login/file2 /etc/file3"

OPTS="$1"
FILES=`echo "$2" | sed -e "s/,/ /g"`
MAX="$3"

cnt=0
for i in $FILES
do
	cnt=`expr $cnt + 1`
	valpath $OPTS $i
	if [ $? -ne 0 ]
	then
		exit 1
	fi
done

if [ "$MAX" != "" -a "$cnt" -gt "$MAX" ]
then
	exit 1
else
	exit 0
fi
