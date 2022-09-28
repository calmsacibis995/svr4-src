#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:xx/FACE3.2/admbin/PS/WHEEL/addwheel.sh	1.1.1.1"
if [ "$3" = "once" ]
	then freq=0
else
	freq=$3
fi
if [ "$2" = "mail" ] ; then
/usr/lib/lpadmin -S$1 -A"mail $LOGNAME" -W$freq -Q$4  1>/dev/null 2>&1
else
/usr/lib/lpadmin -S$1 -A "$2" -W$freq -Q$4  1>/dev/null 2>&1
fi

echo 0
