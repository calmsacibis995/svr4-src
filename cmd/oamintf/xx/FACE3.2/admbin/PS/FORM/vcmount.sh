#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:xx/FACE3.2/admbin/PS/FORM/vcmount.sh	1.1.1.1"

if [ "$1" = "mail" -o "$1" = "none" -o -x "$1" ]
	then true
else
	if [ -f "$1" ]
		then echo "$1" is not an executable file. > /usr/tmp/err.$VPID
	elif [ ! -f "$1" ]
		then echo "$1" does not exist. > /usr/tmp/err.$VPID
	fi
	echo false
fi



