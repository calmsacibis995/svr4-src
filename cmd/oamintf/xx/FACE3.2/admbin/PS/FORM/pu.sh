#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:xx/FACE3.2/admbin/PS/FORM/pu.sh	1.1.1.1"

if [ -z "$1" ]
	then 
	echo "Permitted Users is a Mandatory field. Strike CHOICES." > /usr/tmp/err.$VPID
	echo false
	exit
fi

list=`echo "$1" | tr '\012' ' ' `

awk -F: '{ if ( $3 == 0 || $3 >=100 ) print $1 }' /etc/passwd > /usr/tmp/pass.$VPID
if [ "$1" = "all" -o "$1" = "none" ]
	then echo true
	exit
fi

for user in $list
do
	if [ "$user" = "all"  -o "$user" = "none" ]
		then echo "Cannot Mark All, none and other users; Strike CHOICES for valid choices." > /usr/tmp/err.$VPID
		echo false
		exit
	elif [ "`grep \"^$user$\" /usr/tmp/pass.$VPID`" = "" ]
		then echo "$user is not a valid user name. Strike CHOICES for valid choices." > /usr/tmp/err.$VPID
		echo false
		exit
	fi
done

rm -rf /usr/tmp/pass.$VPID

echo true

