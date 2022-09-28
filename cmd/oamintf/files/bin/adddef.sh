#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)filemgmt:bin/adddef.sh	1.1.3.1"
DEV=$1
RDEV="-"
FS=$2
AUTOMNT=$3
FSTYPE=$4
MNTOPT1=$5
MNTOPT2=$6
if test -b "$DEV"
then
	BDEVICE="$DEV"
else
	BDEVICE=`devattr "$DEV"  bdevice 2>/dev/null`
	if test "$BDEVICE" != ""
	then
		DEV="$BDEVICE"
	fi
fi
RDEVICE=`devattr "$DEV"  cdevice 2>/dev/null`
if test "$RDEVICE" != ""
then
	RDEV="$RDEVICE"
fi
if test "$MNTOPT1" = "read-only"
then
	MNTOPT1="ro"
else
	MNTOPT1="rw"
fi
if test "$MNTOPT2" = "yes"
then
	MNTOPTS="$MNTOPT1,suid"
else
	MNTOPTS="$MNTOPT1,nosuid"
fi
echo $DEV $RDEV $FS $FSTYPE "-" $AUTOMNT $MNTOPTS | awk '{printf("%-17s %-17s %-6s %-6s %-8s %-7s %-8s\n", $1, $2, $3, $4, $5, $6, $7)}' >>/etc/vfstab
