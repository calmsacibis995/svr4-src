#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:xx/FACE3.2/admbin/PS/FORM/depend.sh	1.1.1.1"
if [ ! -z "$1" -a -z "$2"  ]
	then 
	echo "Since field \"Content type\" is empty, the current field must be empty." > /usr/tmp/err.$VPID
	echo 1
elif [  -z "$1" -a ! -z "$2"  ]
	then 
	echo "Since field \"Content type\" is not empty, the current field must not be empty." > /usr/tmp/err.$VPID
	echo 1
	exit
elif [ "$3" !=  "none" ]
	then
	if [ -z "$4" ]
	then
	echo "Since field \"Mount\" is not none, the \"Frequency\" field must not be empty." > /usr/tmp/err.$VPID
	echo 1
	exit
	elif [ -z "$5" ]
	then
	echo "Since field \"Mount\" is not none, the \"Number\" field must not be empty." > /usr/tmp/err.$VPID
	echo 1
	exit
	fi
elif [ "$3" =  "none" ]
	then
	if [ ! -z "$4" ]
	then
	echo "Since field \"Mount\" is none, the \"Frequency\" field must be empty." > /usr/tmp/err.$VPID
	echo 1
	exit
	elif [ ! -z "$5" ]
	then
	echo "Since field \"Mount\" is none, the \"Print requests\" must be empty." > /usr/tmp/err.$VPID
	echo 1
	exit
	fi
fi
echo 0

