#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:xx/FACE3.2/admbin/PS/FORM/allvalid.sh	1.1.1.1"



if [ -z "$1" ]
	then echo "Input must be an integer of 2 digits or less." > /usr/tmp/err.$VPID
	echo false
else 
	/usr/vmsys/admin/PS/FORM/alphanum "$1"
	if [ $? = 0 ]
		then echo true
	else
		echo "$1 is not valid for characters per inch. (See the printer manual.)" > /usr/tmp/err.$VPID
		echo false
	fi
fi





22



#ident	"@(#)oamintf:xx/FACE3.2/admbin/PS/FORM/allvalid.sh	1.1.1.1"

if [ -z "$1" ]
	then echo "Input must be an integer of 2 digits or less." > /usr/tmp/err.$VPID
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




3











if [ "$1" = "mail" -o "$1" = "none" -o -x "$1" ]
	then :
else
	if [ -f "$1" ]
		then echo "$1" is not an executable file. > /usr/tmp/err.$VPID
	elif [ ! -f "$1" ]
		then echo "$1" does not exist. > /usr/tmp/err.$VPID
	fi
	echo false
	exit
fi



if [ ! -z "$2" -a "$1" = "none" ]
	then 
echo "Since field \"Mount form alert \" is empty, the current field must be empty." > /usr/tmp/err.$VPID
	echo false
	exit
elif [ -z "$2" -a "$1" = "none" ]
	then :
else
	if [ "$2" = "once" -o "$2" = "1" -o "$2" = "5" -o "$2" = "30" -o "$2" = "60" ]
	then :
	else
		echo "$2 is not a valid frequency. Strike CHOICES for valid choices." > /usr/tmp/err.$VPID
		echo false
		exit
	fi
	
fi




if [ ! -z "$3" -a "$1" = "none" ]
	then 
echo "Since field \"Mount form alert \" is empty, the current field must be empty." > /usr/tmp/err.$VPID
	echo false
	exit
elif [ -z "$3" -a "$1" = "none" ]
	then : 
else
	if [ "$3" = "1" -o "$3" = "5" -o "$3" = "10" -o "$3" = "20" ]
	then : 
	else
		echo "$3 is not a valid frequency. Strike CHOICES for valid choices." > /usr/tmp/err.$VPID
		echo false
		exit
	fi
	
fi
true
