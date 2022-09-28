#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)bkrs:intftools.d/bkup.sh	1.6.3.3"
# Script to invoke the backup command with appropriate options.
# Parameters are:
#    bkreg table, type of backup, object name, weeks, days, notify, display,
#    estimate volumes, trace mode.

TABLE=$1
BTYPE=$2
ONAME=$3
WEEKS="$4"
DAYS="$5"
NOTIFY=$6
DISPLAY=$7
ESTIMATE=$8
TRACE=$9

SELECT=
OPTS="-t $TABLE"

if [ "$BTYPE" = "interactive" ]
then
	OPTS="$OPTS -i"
fi

if [ "$ONAME" != "all" ]
then
	OPTS="$OPTS -o $ONAME"
fi

if [ "$WEEKS" = "current week" -a "$DAYS" != "today" ]
then
	WEEKS=`get_rotvals -t $TABLE | cut -f2 -d: | sed -e "s/Cweek=//"`
fi

if [ "$DAYS" = "today"  -a "$WEEKS" != "current week" ]
then
	DAYS=`date +%w`
fi

if [ "$WEEKS" = "demand" ]
then
	OPTS="$OPTS -c $WEEKS"
elif [ "$WEEKS" != "current week" -o "$DAYS" != "today" ]
then
	OPTS="$OPTS -c \"$WEEKS:$DAYS\""
fi

# NOTE: the use of LOGNAME here is temporary.  Should get user's real login
# and use it, but for time being, since nsysadm will be root (not setuid, but
# really root), this is the best we can do.
if [ $NOTIFY = yes ]
then
	OPTS="$OPTS -m $LOGNAME"
fi

if [ $DISPLAY = yes ]
then
	OPTS="$OPTS -n"
fi

if [ $ESTIMATE = yes ]
then
	OPTS="$OPTS -e"
fi

if [ $TRACE = yes ]
then
	OPTS="$OPTS -v"
fi

unset OAMBASE
eval backup $OPTS
RC=$?
if [ $RC -eq 0 ]
then
	echo "Backup started successfully."
fi
exit $RC
