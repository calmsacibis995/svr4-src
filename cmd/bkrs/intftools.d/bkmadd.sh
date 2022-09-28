#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)bkrs:intftools.d/bkmadd.sh	1.3.3.1"
# script to add a line to crontab to schedule a reminder to back up
# file systems and data partitions.
# Lines are tagged to allow easy identification for later changes
# or removal.
HOUR=`echo "$1" | cut -f1 -d:`
MIN=`echo "$1" | cut -f2 -d:`
DAYS=`echo "$2" | sed -e "s/  */,/g"`
MONTHS=`echo "$3" | sed -e "s/  */,/g"`
ONAMES=`echo "$4" | sed -e "s/  */,/g"`
FILE=/tmp/bkrs$$

if [ "$DAYS" = "all" ]
then
	DAYS=\*
fi

if [ "$MONTHS" = "all" ]
then
	MONTHS=\*
fi

LINE="$MIN $HOUR * $MONTHS $DAYS /usr/sadm/bkup/bin/bkmsg $ONAMES #bkmsg#"

crontab -l >$FILE
echo "$LINE" >>$FILE
crontab <$FILE 2>/dev/null
RC=$?
rm $FILE
exit $RC
