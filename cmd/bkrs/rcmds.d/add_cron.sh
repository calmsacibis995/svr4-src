#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:rcmds.d/add_cron.sh	1.1.3.1"
task=$1
month=$2
date=$3
day=$4
hour=$5
minute=$6
tmp=/tmp/addcron.$VPID

if [ "$month" = "all" ]
then	month='*'
fi

if [ "$date" = "all" ]
then	date='*'
fi

if [ "$day" = "all" ]
then	day='*'
fi

if [ "$hour" = "all" ]
then	hour='*'
fi

if [ "$minute" = "all" ]
then 	minute='*'
fi

if [ "$task" = "System Backup" ]
then task="echo '\\\n' | /usr/bin/backup -t -c -d /dev/rmt/c0s0"
elif [ "$task" = "Incremental System Backup" ]
then task="echo '\\\n' | /usr/bin/backup -t -p -d /dev/rmt/c0s0"
else task=`cat $task`
fi

crontab -l > $tmp
echo "$minute $hour $date $month $day $task" >> $tmp
crontab $tmp 
rm $tmp
