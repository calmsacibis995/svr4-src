#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)bkrs:intftools.d/bkhsel.sh	1.3.3.1"
# shell invokes bkhistory with selective options to filter
# output.  Note that long form of display does a bkhistory
# followed by a bkhistory -l.
FORM=$1
NAMES="$2"
DATES="$4"
TAGS="$3"
OPTS=
TFILE=/tmp/bkh$$

if [ "$NAMES" != "all" ]
then
	OPTS="-o \"$NAMES\""
fi

if [ "$TAGS" != "all" ]
then
	OPTS="$OPTS -t \"$TAGS\""
fi

if [ "$DATES" != "all" ]
then
	OPTS="$OPTS -d \"$DATES\""
fi

if [ "$FORM" = "long" ]
then
	(eval bkhistory $OPTS && eval bkhistory -l $OPTS) >$TFILE 2>&1
	RC=$?
else
	eval bkhistory $OPTS >$TFILE 2>&1
	RC=$?
fi
echo $TFILE
exit $RC
