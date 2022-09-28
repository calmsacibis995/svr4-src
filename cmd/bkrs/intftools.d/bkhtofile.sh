#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)bkrs:intftools.d/bkhtofile.sh	1.2.3.2"
# script to run bkhistory, redirecting the output to a file.
# Note that stderr is redirected to a temp file so the form can display
# it in a text frame.  The first argument tells how to run bkhistory
# (short form, long form or selectively).  The second argument is whether
# to append to the file or overwrite it.  The third argument is the file
# to be written. Remaining arguments are to be for the selective report.
TYPE=$1
APP=$2
FILE=$3
FORM=$4
NAMES="$5"
TAGS="$6"
DATES="$7"
OPTS=

if [ $APP = append ]
then
	OP=">>"
elif [ $APP = overwrite ]
then
	OP=">"
else
	echo $0: Unknown \"append\" argument \"$APP\".
	exit 1
fi

case $TYPE in
	summary)
		eval bkhistory $OP $FILE 2>/tmp/bkherr$$
		;;

	full)
		eval bkhistory $OP $FILE 2>/tmp/bkherr$$ && bkhistory -l >>$FILE 2>>/tmp/bkerr$$
		;;

	select)
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
			eval bkhistory $OPTS $OP $FILE 2>/tmp/bkherr$$ && bkhistory -l $OPTS >>$FILE 2>>/tmp/bkerr$$
		else
			eval bkhistory $OPTS $OP $FILE 2>/tmp/bkherr$$
		fi
		;;
esac

RC=$?
if [ $RC -ne 0 ]
then
	echo $$
	exit $RC
else
	rm -f /tmp/bkerr$$
	echo ok
	exit 0
fi
