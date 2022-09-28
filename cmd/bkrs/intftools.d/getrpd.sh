#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)bkrs:intftools.d/getrpd.sh	1.1.3.1"
# script returns rotation period value in table given as argument.
# if table doesn't exist, the default period (1) is returned.

TABLE=$1

if [ -f $TABLE ]
then
	PD=`grep "ROTATION=" $TABLE 2>/dev/null`
	if [ $? -ne 0 ]
	then
		echo 1
	else
		echo $PD | sed -e "s/^.ROTATION=//"
	fi
else
	echo 1
fi
