#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)sa:sa1.sh	1.8"
#	sa1.sh 1.5 of 5/8/89
DATE=`date +%d`
ENDIR=/usr/lib/sa
DFILE=/var/adm/sa/sa$DATE
cd $ENDIR
if [ $# = 0 ]
then
	exec $ENDIR/sadc 1 1 $DFILE
else
	exec $ENDIR/sadc $* $DFILE
fi
