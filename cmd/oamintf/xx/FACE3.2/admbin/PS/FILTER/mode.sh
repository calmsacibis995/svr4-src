#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:xx/FACE3.2/admbin/PS/FILTER/mode.sh	1.1.1.1"
opt=`grep "$1" /usr/spool/lp/admins/lp/filter.table | cut -d: -f9`
if [ "$opt" = "" ]
then	echo ""
	exit
else
	if echo "$opt" | grep MODES > /dev/null
	then 	
		opt=`echo "$opt" | sed 's/ //g
					s/,/ /g'`
	
		
		vnum=$2
		set -- "$opt"
		j=1
		for i in $*
		do
			if echo "$i" | grep MODES > /dev/null
			then	opt=`echo $i | cut -d"S" -f2 | cut -d"=" -f1`
				single=`echo $i | cut -d"-" -f2`
				echo $opt>/usr/tmp/m$j.$vnum
				echo $single>/usr/tmp/s$j.$vnum
				j=`expr $j + 1`
			fi
		done
	else	exit
	fi
fi
