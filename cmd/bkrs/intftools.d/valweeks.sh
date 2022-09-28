#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)bkrs:intftools.d/valweeks.sh	1.5.3.2"
TABLE=`echo $3`
if [ "$1" = "demand" ]
then
	exit 0
elif [ "$1" = "$2" ]
then
	exit 0
else
	WEEKS=`echo $1`
	PERIOD=`getrpd $TABLE`
	validweeks "$WEEKS" $PERIOD;a=`echo $?`
echo "weeks is $WEEKS period is $PERIOD and retrun validweeks $a" >/tmp/weeks.out
	exit $a
fi
