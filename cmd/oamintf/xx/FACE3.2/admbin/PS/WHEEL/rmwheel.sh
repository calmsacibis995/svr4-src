#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:xx/FACE3.2/admbin/PS/WHEEL/rmwheel.sh	1.1.1.1"


wheels=`echo $1 | sed 's/,/ /g'`
if [ "$wheels" = "all" ]
	then 
	for wheel in `ls /usr/spool/lp/admins/lp/pwheels`
	do
	/usr/lib/lpadmin -S $wheel -A none 2>/dev/null 1>&2
	done
#if the user select few printwheels 
else
	for wheel in $wheels
	do
	/usr/lib/lpadmin -S $wheel -A none 2>/dev/null 1>&2
	done

fi
