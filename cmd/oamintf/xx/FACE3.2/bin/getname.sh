#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:xx/FACE3.2/bin/getname.sh	1.1.1.1"
if [ -s /etc/.devices/$1 ]
then
	. /etc/.devices/$1
	if [ "$TYPE" = 'Printer' ]
	then echo $NAME
	else echo ""
	fi
else	echo ""
fi
