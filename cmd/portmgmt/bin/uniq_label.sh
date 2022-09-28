#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)portmgmt:bin/uniq_label.sh	1.1.2.1"

# uniq_label - check if the ttylabel selected is uniq

NOTHING=1	# Nothing entered
NOTUNIQ=2	# Not unique
OK=0		

# Nothing entered
test -z "$1" && exit $NOTHING

if grep "^$1[ ]*:" /etc/ttydefs > /dev/null 2>&1
then
	exit $NOTUNIQ
else
	exit $OK
fi
