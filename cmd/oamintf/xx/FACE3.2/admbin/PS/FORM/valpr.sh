#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:xx/FACE3.2/admbin/PS/FORM/valpr.sh	1.1.1.1"
if [ ! -z "$1" -a "$2" = "none" ]
	then 
echo "Since field \"Mount form alert \" is empty, the current field must be empty." > /usr/tmp/err.$VPID
	echo false
elif [ -z "$1" -a "$2" = "none" ]
	then echo true
else
	if [ "$1" = "1" -o "$1" = "5" -o "$1" = "10" -o "$1" = "20" ]
	then
		echo true
	else
		echo "$1 is not a valid frequency. Strike CHOICES for valid choices." > /usr/tmp/err.$VPID
		echo false
	fi
	
fi
