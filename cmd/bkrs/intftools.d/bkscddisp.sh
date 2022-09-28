#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)bkrs:intftools.d/bkscddisp.sh	1.3.3.1"
# Script displays the lines from crontab marked by the argument passed in.
# It is used to display the backup schedule and backup reminder schedule
# lines.
# Note: the exit 0 is required to force FMLI to display the error message in
# case the command fails.

(crontab -l | grep \#${1}\# | /usr/sadm/bkup/bin/cron_parse -m $1) 2>&1
exit 0
