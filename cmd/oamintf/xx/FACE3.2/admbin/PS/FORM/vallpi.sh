#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:xx/FACE3.2/admbin/PS/FORM/vallpi.sh	1.1.1.1"

if [ -z "$1" ]
	then echo "Input must be an integer of 2 digits or less." > /usr/tmp/err.$VPID
	echo false
elif [ "$1" = "0" ] ; then
	echo "$1 is not valid for lines per inch. (See the printer manual.)" > /usr/tmp/err.$VPID
	echo false
else 
	/usr/vmsys/admin/PS/FORM/alphanum "$1"
	if [ $? = 0 ]
		then echo true
	else
		echo "$1 is not valid for lines per inch. (See the printer manual.)" > /usr/tmp/err.$VPID
		echo false
	fi
fi
