#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:xx/FACE3.2/admbin/PS/FILTER/seek.sh	1.1.1.1"
ret=1
if [ ! -s /usr/spool/lp/admins/lp/filter.table ]
then	
	echo $ret
	exit
else
	if [ "$2" = d ]
	then	echo "all" > /usr/tmp/tab.$1
	fi
	cut -d: -f5 /usr/spool/lp/admins/lp/filter.table > /usr/tmp/tab$$
	for i in `cat /usr/tmp/tab$$`
	do
		case $i in
		435_table | 475_table | 455_table | 473_table | HP_table )
			;;
		* )
		echo $i >> /usr/tmp/tab.$1
		ret=0
		esac
	done
	echo $ret
fi
rm -f /usr/tmp/tab$$
