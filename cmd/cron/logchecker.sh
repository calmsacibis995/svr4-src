#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)cron:logchecker.sh	1.6.5.1"

# this command is used to determine if the cron log file is approaching the ulimit
# of the system.  If it is, then the log file will be moved to olog
# this command is executed by the crontab entry 'root'

#set umask
umask 022

# log files
LOG=/var/cron/log
OLOG=/var/cron/olog

# set the high-water mark of the file
MARKER=4
LIMIT=`ulimit`
LIMIT=`expr $LIMIT - $MARKER`

# find the size of the log file (in blocks)
if [ -f $LOG ]
then
	FILESIZE=`du -a $LOG | cut -f1`
else
	exit
fi

# move log file to olog file if the file is too big
if [ $FILESIZE -ge $LIMIT ]
then
	cp $LOG $OLOG
	>$LOG
	chgrp bin $OLOG
fi

