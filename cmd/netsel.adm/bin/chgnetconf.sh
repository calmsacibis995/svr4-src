#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)netsel.adm:bin/chgnetconf.sh	1.1.3.1"

# chgnetconf - modify /etc/netconfig

OK=0		# everything is ok
NOCHANGE=1	# fields are not changed
NOTHING=2	# nothing entered

# nothing entered
test -z "$1" && test -z "$2" && exit $NOTHING

if [ $2 = "Yes" ]
then
	newflag="v"
else
	newflag="-"
fi
if [ $5 != $newflag -o $1 != $3 ]
then
	sed "s/$1[	 ]*$4[	 ]*$5/$3	$4	$newflag/" /etc/netconfig > /usr/tmp/list
	exitcode=$?
	cp /usr/tmp/list /etc/netconfig
	rm /usr/tmp/list
	exit $exitcode
else
	exit $NOCHANGE
fi
