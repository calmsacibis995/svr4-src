#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:rcmds.d/runstore2.sh	1.1.2.1"
FLPINFO=$1
TMPFILE=$2
FILES=
if test -s $TMPFILE
then
	for i in `cat $TMPFILE`
	do
		if grep "^D	$i$" $FLPINFO > /dev/null
		then
			FILES="$FILES :$i/*:" 
		else
			FILES="$FILES :$i:"
		fi
	done
	shift 2
	/usr/sadm/sysadm/bin/setquote -c "/usr/bin/restore $* $FILES"
else
	echo "No files or directories were MARKed for restore."
	echo "The MARK function key must be used to select files"
	echo "or directories to be restored."
fi
rm -f $TMPFILE
rm -f $FLPINFO
