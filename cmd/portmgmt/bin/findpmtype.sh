#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)portmgmt:bin/findpmtype.sh	1.2.2.1"

# findpmtype - return the type of port monitor
#	- $1 - a flag, p or t
#	- $2 - if $1 is p, $2 is the tag of port monitor
#	       if $1 is t, $2 is the type of the port monitor

NOTHING=-1	# nothing entered
NOTEXIST=-2	# not exist
UNKNOWN=0
LISTEN=1
TTYMON=2

# nothing entered
test -z "$1" && exit $NOTHING
test -z "$2" && exit $NOTHING

case "$1"
in
	p) 	type=`/usr/sbin/sacadm -L | grep $2 | cut -d: -f2`
		if test -z "$type"
		then
			exit $NOTEXIST
		fi;;
	t) 	type=$2;;
esac
case "$type"
in
	ttymon) exit $TTYMON;;
	listen) exit $LISTEN;;
	*)      exit $UNKNOWN;;
esac
