#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:xx/FACE3.2/admbin/PS/FILTER/chkmode.sh	1.1.1.1"
if [ "$1" = "" -a "$2" = "" ]
then	echo 0
elif	[ "$1" = "" -a "$2" != "" ]
then 	echo 1
elif	[ "$1" != "" -a "$2" = "" ]
then	echo 2
else	echo 0
fi
