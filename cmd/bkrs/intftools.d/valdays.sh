#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)bkrs:intftools.d/valdays.sh	1.4.3.1"
# script passes one comma-separated list to the validation routine
if [ "$1" = "$2" ]
then
	exit 0
else
	DAYS=`echo "$1" | sed -e "s/  */,/g"`
	validdays $DAYS
	exit $?
fi
exit 0
