#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:xx/FACE3.2/admbin/PS/FILTER/mitem.sh	1.1.1.1"
if echo "$1" | grep any > /dev/null
then	echo Y
	exit
else	
	a=`echo "$1" | tr "," " "`
	if [ "$2" = type ]
	then
		cut -f1 /usr/vmsys/admin/PS/PORTSET/database > /usr/tmp/type$$
		sfile=/usr/tmp/type$$
	else	sfile=/usr/tmp/pname.$3
	fi
	set -- "$a"
	for i in $*
	do
		if grep "^$i$" $sfile > /dev/null
		then	:
		else	echo $i
			exit
		fi
	done
	echo Y
fi
rm -f /usr/tmp/type$$
