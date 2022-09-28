#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)portmgmt:bin/lsopts.sh	1.4.2.1"

# lsopts - form a string for the nlsadmin command options for listener

opts=""

if [ "$1" = "Spawn a service" ]
then
	opts="$opts -c \\\"$2\\\""
else
	opts="$opts -o $2"
fi

if test -n "$3"
then
	opts="$opts -p $3"
fi

if test -n "$4"
then
	addr=`echo $4 | sed 's/\\\\/\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\/'`
	opts="$opts -A $addr"
fi

if test -n "$5"
then
	ver=`echo $5 | tr , :`
	opts="$opts -p $ver"
fi

echo -m\"\`nlsadmin "$opts" \`\" 
