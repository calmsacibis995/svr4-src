#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)bkrs:intftools.d/bkmsg.sh	1.1.3.1"
# script prints backup reminder message to standard output.  It
# is invoked with the list of file systems to remind about (comma-separated)
# or the keyword "all".

FSYS=$1

if [ "$FSYS" = "all" ]
then
	echo "Reminder: it is time to back up all file systems and data partitions."
else
	PRFSYS=`echo $FSYS | sed -e "s/,/, /g"`
	echo Reminder: it is time to back up "$PRFSYS".
fi
exit 0
