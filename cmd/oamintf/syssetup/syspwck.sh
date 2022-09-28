#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:syssetup/syspwck.sh	1.1.3.1"
syslogs=`sed -n '/^[^:]*:[^:]*:.\{1,2\}:/s/:.*//p' /etc/passwd | sort -u`

# get system logins which do not have a password
	for sys in $syslogs
	do
		if passwd -s $sys 2>/tmp/passwd.err | grep "^$sys  *LK" >/dev/null
		then
			echo $sys >> /tmp/syslogins
		fi
	done


if [ -s /tmp/syslogins ] && [ ! -s /tmp/passwd.err ]
then exit 0
else exit 1
fi


