#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)portmgmt:bin/uniq_svctag.sh	1.1.2.1"

# uniq_svctag - check to see if the svctag is uniq
# Usage: uniq_svctag 	p pmtag svctag | t pmtype svctag

ARGNUM=1	# Num. of args incorrect
NOTUNIQ=2	# Not unique
OK=0		

# Nothing entered
test -z "$1" && exit $ARGNUM
test -z "$2" && exit $ARGNUM
test -z "$3" && exit $ARGNUM

case "$1" in
	p) found=`pmadm -L -p "$2"| cut -d: -f3 | grep "^$3$"`
		;;
	t) found=`pmadm -L -t "$2"| cut -d: -f3 | grep "^$3$"`
		;;
	*) echo "bad argument."
	   exit $ARGNUM
		;;
esac
if test -n "$found"
then
	exit $NOTUNIQ
else
	exit $OK
fi
