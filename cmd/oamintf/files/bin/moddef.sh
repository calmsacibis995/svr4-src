#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)filemgmt:bin/moddef.sh	1.1.5.1"
NEWDEV=$1
NEWMOUNTP=$2
FSTYPE=$3
AUTOMNT=$4
RW=$5
SUID=$6
BDEV=$7
MOUNTP=$8
> /tmp/vfstab
RDEV=`devattr "$NEWDEV" cdevice 2> /dev/null`
if [ $? != 0 ]
then
	RDEV=-
fi
if [ ! -b $BDEV ]
then
	BDEV=`devattr "$BDEV" bdevice 2>/dev/null`
fi
if [ ! -b $NEWDEV ]
then
	NEWDEV=`devattr "$NEWDEV" bdevice 2>/dev/null`
fi
MNTOPTS="-"
if [ "$SUID" = "yes" ]
then
	MNTOPTS="suid"
else
	MNTOPTS="nosuid"
fi
if [ "$RW" = "read-only" ]
then
	if [ "t$MNTOPTS" = "t-" ]
	then
		MNTOPTS="ro"
	else
		MNTOPTS="$MNTOPTS,ro"
	fi
fi
while read bdev rdev mountp fstype fsckpass automnt mntopts
do
	if test "$BDEV" = "$bdev" -a "$MOUNTP" = "$mountp"
	then
		echo ${NEWDEV} ${RDEV} ${NEWMOUNTP} ${FSTYPE} "-" ${AUTOMNT} ${MNTOPTS} | awk '{printf("%-17s %-17s %-6s %-6s %-8s %-7s %-8s\n", $1, $2, $3, $4, $5, $6, $7)}' >> /tmp/vfstab
		continue
	fi
	echo ${bdev} ${rdev} ${mountp} ${fstype} ${fsckpass} ${automnt} ${mntopts} | awk '{printf("%-17s %-17s %-6s %-6s %-8s %-7s %-8s\n", $1, $2, $3, $4, $5, $6, $7)}' >> /tmp/vfstab
done < /etc/vfstab
cp /tmp/vfstab /etc/vfstab
