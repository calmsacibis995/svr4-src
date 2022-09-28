#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:xx/FACE3.2/admbin/PS/FILTER/option.sh	1.1.1.1"
opt=`grep "$1" /usr/spool/lp/admins/lp/filter.table | cut -d: -f9`
if [ "$opt" = "" ]
then	echo ""
	exit
else
	if echo "$opt" | grep "$2" > /dev/null
	then 	
		opt=`echo "$opt" | sed 's/ //g
					s/*//g
					s/,/ /g'`
		single="$2"
	
		
		set -- "$opt"
		for i in $*
		do
			if echo "$i" | grep "$single" > /dev/null
			then	opt=`echo $i | cut -d"-" -f2`
				echo $opt
				exit
			fi
		done
	else	exit
	fi
fi
