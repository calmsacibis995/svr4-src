#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:intftools.d/change.sh	1.5.3.2"
# Program invokes bkreg with -e option to edit a line in a backup register.

TABLE="$1"
TAG="$2"
TPRI="$3"
ONAME="$4"
ODEV="$5"
OMNAMES="$6"
WEEKS="$7"
DAYS="$8"
METHOD="$9"

shift 9
MOPTS="$1"
DEPEND="$2"
DGROUP="$3"
DDEV="$4"
DCHARS="$5"
DMNAMES="$6"

SELECT=
OPTS=

if [ "$WEEKS" = "all" ]
then
	PD=`getrpd $TABLE`
	WEEKS="1-$PD"
fi

if [ "$DAYS" = "all" ]
then
	DAYS="0-6"
fi

if [ "$WEEKS" = "demand" ]
then
	SELECT="$WEEKS"
else
echo $WEEKS $DAYS >/tmp/dft1
	SELECT="$WEEKS:$DAYS"
fi

if [ "$TPRI" = "" ]
then
	PRI="";
else
	PRI="-P $TPRI"
fi

DEST="$DGROUP:$DDEV:$DCHARS:$DMNAMES"

ORIG="$ONAME:$ODEV:$OMNAMES"

if [ -n "$MOPTS" ]
then
	OPTS="-b \"$MOPTS\""
fi

if [ -n "$DEPEND" ]
then
	OPTS="$OPTS -D \"$DEPEND\""
fi

eval bkreg -e "$TAG" -t "$TABLE" -c \"$SELECT\" -m "$METHOD" -d \"$DEST\" -o \"$ORIG\" "$PRI"  "$OPTS" 2>&1

RVAL=$?
if [ $RVAL -eq 0 ]
then
	echo Entry successfully edited in backup register.
fi
exit $RVAL
