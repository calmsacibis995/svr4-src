#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:xx/FACE3.2/admbin/PS/FILTER/delete.sh	1.1.1.1"
if [ "$1" = all ]
then	
#	/usr/lib/lpfilter -fall -x
	cut -d: -f5 /usr/spool/lp/admins/lp/filter.table > /usr/tmp/base$$
	for i in `cat /usr/tmp/base$$`
	do
		case $i in
		435_table | 475_table | 455_table | 473_table | HP_table )
			;;
	* )
		/usr/lib/lpfilter -f $i -x
			;;
		esac
	done
	rm -f /usr/tmp/base$$
else
	list=`echo "$1" | tr "," " "`
	set -- "$list"
	for i in $*
	do
		/usr/lib/lpfilter -f $i -x
	done
fi
