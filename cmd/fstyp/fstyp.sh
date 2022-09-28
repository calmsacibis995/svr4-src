#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)fstyp:fstyp.sh	1.4.5.1"
#
#	Determine the fs identifier of a file system.
#
#!	chmod +x ${file}
USAGE="Usage: fstyp [-v] special"
NARGS=`echo $#`

if [ $NARGS -eq 0 ]
then
	echo "$USAGE" >&2
	exit 2
fi
while getopts v? c
do
	case $c in
	 v) VFLAG="-"$c;;
	\?) echo "$USAGE" >&2
	    exit 2;;
	esac
done
shift `expr $OPTIND - 1`

if [ "$VFLAG" ]
then
	if [ $NARGS -gt 2 ]
	then
		echo "$USAGE" >&2
		exit 2
	fi
else
	if [ $NARGS -gt 1 ]
	then
		echo "$USAGE" >&2
		exit 2
	fi
fi
	


SPEC=$1
if [ "$SPEC" = "" ]
then
	echo "$USAGE" >&2
	exit 2
fi
if [ ! -r $SPEC ]
then
	echo "fstyp: cannot stat or open <$SPEC>" >&2
	exit 1
fi

if [ \( ! -b $SPEC \) -a \( ! -c $SPEC \) ]
then
	echo "fstyp: <$SPEC> not block or character special device" >&2
	exit 1
fi

#
#	Execute all heuristic functions /etc/fs/*/fstype 
#	or /usr/lib/fs/*/fstyp and
#	return the fs identifier of the specified file system.
#

CNT=0 

if [ -d /usr/lib/fs ]
then
	DIR=/usr/lib/fs
else
	DIR=/etc
fi

for f in $DIR/*/fstyp
do
	$f $VFLAG $SPEC >&1
	if [ $? -eq 0 ]
	then
		CNT=`expr ${CNT} + 1`
	fi
done

if [ ${CNT} -gt 1 ]
then
	echo "Unknown_fstyp (multiple matches)" >&2
	exit 2
elif	[ ${CNT} -eq 0 ]
then
	echo "Unknown_fstyp (no matches)" >&2
	exit 1
else
	exit 0
fi
