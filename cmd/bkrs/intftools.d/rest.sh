#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:intftools.d/rest.sh	1.8.3.2"
# script sets up command line and calls restore utility
TYPE="$1"
NAMES="$2"
TARGET="$3"
DATE="$4"
DISPLAY=$5
NOTIFY=$6
TRACE=$7
TFILE=$8
OPTS=
TEMPF=/tmp/rest_$$

if [ "$TARGET" != "" ]
then
	OPTS="$OPTS -o $TARGET"
fi

OPTS="$OPTS -d \"$DATE\""

if [ "$DISPLAY" = "yes" ]
then
	OPTS="$OPTS -n"
fi

if [ "$NOTIFY" = "yes" ]
then
	OPTS="$OPTS -m"
fi

if [ "$TRACE" = "yes" ]
then
	OPTS="$OPTS -v"
fi

if [ "$TYPE" = "file" ]
then
	CMD=urestore
	OPTS="$OPTS -F"
elif [ "$TYPE" = "directory" ]
then
	CMD=urestore
	OPTS="$OPTS -D"
elif [ "$TYPE" = "file system" ]
then
	CMD=restore
	OPTS="$OPTS -S"
elif [ "$TYPE" = "data partition" ]
then
	CMD=restore
	OPTS="$OPTS -P"
else
	echo "Error - type of restore must be file, directory, file system or"
	echo "data partition."
	exit 1
fi

echo "Attempting automatic restore from online archive...\n"
eval $CMD $OPTS "$NAMES" >$TFILE
RC=$?
grep -v "Attempting" $TFILE > $TEMPF 2>/dev/null
mv $TEMPF $TFILE >/dev/null 2>&1
#echo $TFILE
exit $RC
