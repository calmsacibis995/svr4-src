#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)filemgmt:bin/defdev.sh	1.1.3.1"
while read fsys dummy 
do
	case $fsys in
	'#'* | '')
		continue;;
        '-') 
		continue
	esac

	echo "${fsys}"
done < /etc/vfstab 
