#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:syssetup/gettz.sh	1.1.2.1"

#gettz
# gets proper time zone for screen

tz=`echo $TZ | cut -c1-4`
case ${tz} in
	'GMT0')	 timez='Greenwich' ;;
	'AST4')	 timez='Atlantic' ;;
	'EST5')	 timez='Eastern' ;;
	'CST6')	 timez='Central' ;;
	'MST7')	 timez='Mountain' ;;
	'PST8')	 timez='Pacific' ;;
	'YST8')	 timez='Yukon' ;;
	'AST1')	 timez='Alaska' ;;
	'BST1')	 timez='Bering' ;;
	'HST1')	 timez='Hawaii' ;;
	'*')	 timez=' ' ;;
esac
echo $timez >/tmp/gettz
