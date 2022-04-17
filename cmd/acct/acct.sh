#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)acct:acct.sh	1.3.5.1"
state=$1
set `who -r`
if [ $8 != "0" ]
then
	exit
fi
case $state in

'start')
	if [ $9 = "2" -o $9 = "3" ]
	then
		exit
	fi
	echo "Starting process accounting"
	/usr/lib/acct/startup
	;;
'stop')
	echo "Stopping process accounting"
	/usr/lib/acct/shutacct
	;;
esac
