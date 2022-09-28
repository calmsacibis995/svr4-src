#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:xx/FACE3.2/admbin/PS/FORM/getall.sh	1.1.1.1"
/usr/vmsys/admin/PS/FORM/gform $1 $2 
exist=`grep "^$1:" /usr/vmsys/OBJECTS/PS/FORM/alnames 2>/dev/null | cut -d' ' -f2` 
if [ ! -z "$exist" ]
then
	echo pattern=$exist >> /usr/tmp/form.$VPID
fi
if [ -s /usr/spool/lp/admins/lp/forms/$1/allow ]
	then
	permit=`awk '{print $1 }' /usr/spool/lp/admins/lp/forms/$1/allow `
elif [ -f /usr/spool/lp/admins/lp/forms/$1/deny ]
	then
	echo 
	permit=all 
elif [ ! -f /usr/spool/lp/admins/lp/forms/$1/deny ]
	then
	permit=none 
fi
echo users=$permit >> /usr/tmp/form.$VPID
