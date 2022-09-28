#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)portmgmt:bin/sacopts.sh	1.3.2.1"

# sacopts - form a string containing sac options

opts=""
if [ "$1" = No ]
then
	opts="$opts -fx"
fi

if [ "$2" = DISABLED ]
then
	opts="$opts -fd"
fi

if [ "$3" -gt 0 ]
then
	opts="$opts -n $3"
fi

if test -n "$4"
then
	opts="$opts -z $4"
fi

if test -n "$5"
then
	opts="$opts -y \"$5\""
fi

echo "$opts" 
