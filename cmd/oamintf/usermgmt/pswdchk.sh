#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:usermgmt/pswdchk.sh	1.3.3.1"

#pswdchk

# for running password command to modify aging information

pwlogin=$1
pwchoice=$2
pwmax=$3

if [ "$4" = " " ] || [ "$4" = "" ]
then
	pwmin="-n 0"
else
	pwmin="-n $4"
fi

if [ "$5" = " " ] || [ "$5" = "" ]
then
	pwwarn="-w 0"
else
	pwwarn="-w $5"
fi

if [ "$pwmax" = " " ] || [ "$pwmax" = "" ]
	then pwmax="-1"
fi

if [ "$pwmax" = "-1" ] 
then
	pwmin=
	pwwarn=
fi


case "$pwchoice" in
	
	'lock' )	/usr/bin/passwd -l -x $pwmax $pwmin $pwwarn $pwlogin >/tmp/pswdchk 2>&1 
			;;

	'no password' )	/usr/bin/passwd -d -x $pwmax $pwmin $pwwarn $pwlogin >/tmp/pswdchk 2>&1 
			;;

	'password' | 'new password' )	/usr/bin/passwd -x $pwmax $pwmin $pwwarn $pwlogin >/tmp/pswdchk 2>&1 

esac
