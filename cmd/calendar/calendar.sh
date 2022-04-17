#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)calendar:calendar.sh	1.9"
#	calendar.sh - calendar command, uses /usr/lib/calprog

PATH=/usr/bin
_tmp=/tmp/cal$$
trap "rm -f ${_tmp} /tmp/calendar.$$; trap '' 0; exit" 0 1 2 13 15
/usr/lib/calprog > ${_tmp}
case $# in
0)	if [ -f calendar ]; then
		egrep -f ${_tmp} calendar
	else
		echo $0: `pwd`/calendar not found
	fi;;
*)	cat /etc/passwd | \
		sed 's/\([^:]*\):.*:\(.*\):[^:]*$/_dir=\2 _user=\1/' | \
		while read _token; do
			eval ${_token}	# evaluates _dir= and _user=
			if [ -s ${_dir}/calendar ]; then
				egrep -f ${_tmp} ${_dir}/calendar 2>/dev/null \
					> /tmp/calendar.$$
				if [ -s /tmp/calendar.$$ ]; then
					mail ${_user} < /tmp/calendar.$$
				fi
			fi
		done;;
esac
exit 0
