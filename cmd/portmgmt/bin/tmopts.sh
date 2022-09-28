#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)portmgmt:bin/tmopts.sh	1.3.2.1"

opts=""

if [ "$1" = No ]
then
	opts="$opts -h"
fi

if [ "$2" = Yes ]
then
	opts="$opts -c"
fi

if [ "$3" = Yes ]
then
	opts="$opts -b"
fi

if [ "$4" = Yes ]
then
	opts="$opts -r $5"
fi

if [ "$6" -gt 0 ]
then
	opts="$opts -t $6"
fi

if [ "$7" != "login: " -a "$7" != login: ]
then
	opts="$opts -p \\\"$7 \\\""
fi

if test -n "$8"
then
	opts="$opts -m $8"
fi

if test -n "$9"
then
	opts="$opts -i \\\"$9\\\""
fi

echo -m\"\`ttyadm "$opts" \\ 
