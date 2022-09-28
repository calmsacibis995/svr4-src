#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)bkrs:intftools.d/rsschg.sh	1.4.3.1"
# Shell invokes rsoper command to complete or cancel jobs.
# The first argument is the operation (complete or cancel),
# the second is the jobid of the job to be completed or cancelled.

OPERATION="$1"
JOBID="$2"

if [ "$OPERATION" = 'cancel' ]
then
	OP=-c
elif [ "$OPERATION" = 'complete' ]
then
	OP=-r
else
	echo Illegal operation $OPERATION specified.
	exit 1
fi

rsoper $OP $JOBID 2>&1
exit $?
