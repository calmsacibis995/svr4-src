#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)filemgmt:bin/ifdiskette.sh	1.1.2.1"
# usage: ifdiskette special
# returns true if diskette else returns false
DEVICE=$1
DISKETTE=`devattr diskette1 bdevice 2> /dev/null`
FLOPPY=`devattr $DEVICE bdevice 2> /dev/null`
if [ "$DISKETTE" = "$FLOPPY" ]
then
 	echo "true"
	exit 0
else
	echo "false"
	exit 1
fi
