#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)face:src/xx/chksys.sh	1.2"

if [ -z "$1" ]
then	echo "This is a mandatory field. Strike CHOICES for choices."
	exit
fi
if grep "^$1:" /etc/passwd > /dev/null
then	echo 0
else	
	if echo "$1"|grep ! > /dev/null
	then
		uuname>/tmp/uu.$$
		other=`echo "$1"|cut -d! -f1`
		if [ ! -s /tmp/uu.11 ]
		then	echo "'No other mail system has been set up. See \"Mail Setup\" for mail systems setup.'"
		elif 	grep "^$other$" /tmp/uu.$$ > /dev/null
		then	echo 0
		else	echo "'\"'$other'\" is not set up. See \"Mail Setup\" for mail systems setup.'"
		fi
		rm -f /tmp/uu.$$
	else
		echo "This is not a valid login on your system. Strike CHOICES for valid choices."
	fi
fi
