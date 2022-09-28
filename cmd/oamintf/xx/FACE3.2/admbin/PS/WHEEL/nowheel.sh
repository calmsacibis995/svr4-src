#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:xx/FACE3.2/admbin/PS/WHEEL/nowheel.sh	1.1.1.1"


if [ -z "$1" ]
	then echo "\"Printwheel Name\" is a mandatory field. Type a printwheel name." > /usr/tmp/err.$VPID
	echo false
elif [ -d /usr/spool/lp/admins/lp/pwheels/"$1" ]
	then echo "$1 is an existing printwheel. Type another printwheel name." > /usr/tmp/err.$VPID
	echo false
elif [ "$1" = "all" -o "$1" = "any" -o "$1" = "none" ]
	then
		echo false
	echo "Printwheel names \"all\", \"any\" and \"none\" are reserved names." > /usr/tmp/err.$VPID
else
       /usr/vmsys/admin/PS/CONFIG/alphanum "$1"
	if [ $? = 1 ]
	then
	echo "Name is limited to 8 characters, alphanumeric or \"_\"." > /usr/tmp/err.$VPID
		echo false
	else
		echo true
	fi
fi

