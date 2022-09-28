#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)bkrs:intftools.d/valrsids.sh	1.2.3.1"
#script to validate a list of restore jobids (blank- or comma-separated)
# Called with one argument: the list of restore jobids.
# They are validated for form only!

JOBIDS=`echo "$1" | sed -e "s/,/ /g"`

for i in $JOBIDS
do
	rsid $i
	if [ $? -ne 0 ]
	then
		exit 1
	fi
done
exit 0
