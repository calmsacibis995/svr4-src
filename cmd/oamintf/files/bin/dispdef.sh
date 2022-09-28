#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)filemgmt:bin/dispdef.sh	1.1.3.1"
set -x
DEV=$1
FS=$2
entry="false"
/bin/rm /tmp/vfstabdisp 2>/dev/null
echo "Mount Device        Filesystem      Automount     Type   Mount Options\n------------        ----------      ---------     ----   -------------\n" >> /tmp/vfstabdisp

if [ "$DEV" = "ALL" -a "$FS" = "ALL" ]
then
	while read bdev rdev mountp fstype fsckpass automnt mntopts
	do
		case $bdev in
		'#'* | '' )
			continue;;
		'-')
			continue
		esac
		echo ${bdev} ${mountp} ${automnt} ${fstype} ${mntopts} | awk '{printf("%-20s%-15s%10s%9s%16s\n", $1, $2, $3, $4, $5)}' >> /tmp/vfstabdisp
	done < /etc/vfstab
	exit 0
fi
if [ "$DEV" = "ALL" ]
then
	/usr/bin/egrep "[ 	]+$FS[ 	]+" /etc/vfstab  | awk '{printf("%-20s%-15s%10s%9s%16s\n", $1, $3, $6, $4, $7)}' >> /tmp/vfstabdisp
	exit 0
fi
if [ "$FS" = "ALL" ]
then
	/usr/bin/egrep "^$DEV[ 	]+" /etc/vfstab  | awk '{printf("%-20s%-15s%10s%9s%16s\n", $1, $3, $6, $4, $7)}' >> /tmp/vfstabdisp
	exit 0
fi
	BDEVICE=`devattr "$DEV"  bdevice 2>/dev/null`
	if test "$BDEVICE" != ""
	then
		DEV="$BDEVICE"
	fi
	exec < /etc/vfstab
	while read bdev rdev mountp fstype fsckpass automnt mntopts
	do
		case $bdev in
		'#'* | '' )
			continue;;
		'-')
			continue;;
		esac
		if test "$DEV" != "$bdev" -o "$FS" != "$mountp"
		then
			continue
		fi
		entry="true"
		echo ${bdev} ${mountp} ${automnt} ${fstype} ${mntopts} | awk '{printf("%-20s%-15s%10s%9s%16s\n", $1, $2, $3, $4, $5)}' >> /tmp/vfstabdisp
		exit 0
	done
	if [ "$entry" = "false" ]
	then
	 	echo "Defaults for this file system do not exist." > /tmp/vfstabdisp
	fi
	exit 0

