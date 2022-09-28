#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)bkrs:intftools.d/chgmsg.sh	1.3.3.1"
# Script to change the values in a line of the backup reminder schedule.

#set -x
TFILE1=$1
LINE=$2
HOUR=`echo "$3" | cut -f1 -d:`
MIN=`echo "$3" | cut -f2 -d:`
DAYS=`echo "$4" | sed -e "s/  */,/g"`
MONTHS=`echo "$5" | sed -e "s/  */,/g"`
ONAMES=$6
NEWFILE=/tmp/bkrs$$
NEWCRON=/tmp/bkcron$$

if [ "$DAYS" = "all" ]
then
	DAYS=\*
fi

if [ "$MONTHS" = "all" ]
then
	MONTHS=\*
fi

OPTS="-t $TABLE"

NEWLINE="$MIN $HOUR * $MONTHS $DAYS /usr/sadm/bkup/bin/bkmsg $ONAMES #bkmsg#"

sed -e "${LINE}s:^.*$:${NEWLINE}:" $TFILE1 >$NEWFILE
RC=$?
if [ $RC -eq 0 ]
then
	mv $NEWFILE $TFILE1
	crontab -l | grep -v \#bkmsg\# | cat - $TFILE1 >$NEWCRON
	crontab <$NEWCRON 2>/dev/null
	RC=$?
	rm $NEWCRON
fi
exit $RC

