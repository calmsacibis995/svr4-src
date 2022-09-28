#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:devices/devtabcmd.sh	1.1.3.1"
CMD=`devattr $1 $2`
if [ $? -ne 0 ]
then
	exit 1
fi
$CMD 2>/dev/null
if [ $? -ne 0 ]
then
	exit 1
else
	exit 0
fi
