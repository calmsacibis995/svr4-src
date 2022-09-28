#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:xx/FACE3.2/admbin/PS/FORM/valcon.sh	1.1.1.1"
if [ ! -z "$1" -a -z "$2" ]
	then 
echo "Since field \"Alignment pattern file\" is empty, the current field must be empty." > /usr/tmp/err.$VPID
	echo false
elif [ -z "$1" -a -z "$2" ]
	then echo true
elif [ -z "$1" -a ! -z "$2" ]
	then
	echo "Content type is limited to 14 characters,alphanumeric and \"_\"." > /usr/tmp/err.$VPID
	echo false
else
	/usr/vmsys/admin/PS/CONFIG/alphanum "$1"
	if [ $? = 1 ]
	then
	echo "Content type is limited to 14 characters,alphanumeric and \"_\"." > /usr/tmp/err.$VPID
	echo false
	else
	echo true
	fi
	
fi
