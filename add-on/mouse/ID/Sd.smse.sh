#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)mouse:ID/Sd.smse.sh	1.1"

pid=`/usr/bin/ps -e | /usr/bin/grep mousemgr | /usr/bin/sed -e 's/^  *//' -e 's/ .*//'`
	if [ "${pid}" != "" ]
	then
		/usr/bin/kill ${pid}
	fi
