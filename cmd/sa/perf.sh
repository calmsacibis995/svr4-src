#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)sa:perf.sh	1.4.3.1"
MATCH=`who -r|grep -c "[234][	 ]*0[	 ]*[S1]"`
if [ ${MATCH} -eq 1 ]
then
	su sys -c "/usr/lib/sa/sadc /var/adm/sa/sa`date +%d`"
fi
