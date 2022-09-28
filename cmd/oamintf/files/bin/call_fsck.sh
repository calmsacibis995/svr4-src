#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)filemgmt:bin/call_fsck.sh	1.1.3.1"
/sbin/fsck -F $1 -m $2 1>/tmp/make 2>&1
if [ $? -eq 0 ]
then
	echo 0
else
	echo 1
fi
