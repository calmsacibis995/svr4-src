#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)bkrs:intftools.d/valrpath.sh	1.1.3.1"
# script to validate path name for a restore job.
# A null path is legal (restores to original object's path).
# A file name is only legal if the type of restore is "file", otherwise
# a directory is required.  The directory will be created if it doesn't
# already exist.
# Script must be called with $1 = restore type, $2 = target path.
TYPE="$1"
TARGET=`echo "$2" | sed -e "s/  *//"`

if [ "$2" = "" ]
then
	exit 0
fi
if [ "$1" = "file" ]
then
	valpath -aw $2
else
	valpath -aytw $2
fi
exit $?
