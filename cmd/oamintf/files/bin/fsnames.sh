#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)filemgmt:bin/fsnames.sh	1.1.3.1"
DEV=$1
REMorMOD=$2
if test -b $DEV
then
	BDEVICE=$DEV
else
	BDEVICE=`devattr "$DEV" bdevice 2>/dev/null`
	if test "$BDEVICE" = ""
	then
		BDEVICE=$DEV
	fi
fi
while read device dummy mountp dummy2
do
	if test "$BDEVICE" = "$device"
	then
		echo "$mountp"
	fi
done < /etc/vfstab
if [ "$REMorMOD" = "REM" ]
then
	all=ALL
	echo "$all"
fi
