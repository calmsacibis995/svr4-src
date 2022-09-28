#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)bkrs:intftools.d/bkschg.sh	1.5.3.1"
# Shell invokes backup command to suspend, resume or cancel jobs.
# The first argument is the operation (suspend, resume or cancel),
# the second is either a list of jobids to be operated upon
# or the keyword "all".  The third argument is a userid
# to whose backup jobs the operation applies or 'all'.

OPERATION="$1"
CONTROL="$2"
USERORJOB="$3"

if [ $CONTROL = user ]
then
	if [ "$USERORJOB" = "all" ]
	then
		OPTS=-A
	else
		OPTS="-u $USERORJOB"
	fi
else
	OPTS="-j $USERORJOB"
fi

if [ $OPERATION = suspend ]
then
	CHGED="suspended"
	OPTS="$OPTS -S"
elif [ $OPERATION = resume ]
then
	CHGED="resumed"
	OPTS="$OPTS -R"
elif [ $OPERATION = cancel ]
then
	CHGED="cancelled"
	OPTS="$OPTS -C"
else
	exit 2
fi

unset OAMBASE
backup $OPTS 2>&1
exit $?
