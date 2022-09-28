#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)filemgmt:bin/filsys.sh	1.1.2.1"
while read bdev cdev mountp dummy
do
	case $bdev in
	'#'*| ' ')
		continue;;
	esac
	if [ "$mountp" !=  "/usr" -a "$mountp" != "/" ]
	then
		echo $mountp
	fi
done < /etc/vfstab
