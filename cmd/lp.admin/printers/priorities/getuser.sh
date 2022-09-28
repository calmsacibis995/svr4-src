#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)lp.admin:printers/priorities/getuser.sh	1.1"
# getuser.c : this replaces awk 
#             if the user list is priorities to remove, then
#		it only lists users that have priority limits.
#
#		If the list is to set priorities, it lists users
#		with login ids over 100.

if [ "$1" = "users" ];
then
	for i in `cat /etc/passwd`
	do
		if [ "`echo $i |cut -f3 -d:`" -ge "100" ]
			then echo $i | cut -f1 -d:;
		fi;
	done
	echo all;
elif [ "$1" = "removep" ];
then
	#cut -f2 -d"	" /etc/lp/users | sed -e /0/d;
	cat $remove_1;
	echo all;
	rm -f $remove_1
fi
