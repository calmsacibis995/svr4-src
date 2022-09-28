#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)filemgmt:bin/getdeffs.sh	1.1.2.1"
echo "ALL"
while read fsys dummy  fsname dummy
do
	case $fsys in
	'#'* | '')
		continue
	esac
	case $fsname in
	'-')
		continue
	esac

	echo "${fsname}"
done < /etc/vfstab 
