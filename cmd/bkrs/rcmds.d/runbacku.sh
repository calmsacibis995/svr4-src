#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:rcmds.d/runbacku.sh	1.1.3.1"
TMPFILE=$1
if test -s $TMPFILE
then
	grep "all" $TMPFILE > /dev/null
	if test $? = 0
	then 
		echo "all" > $TMPFILE
	fi
	shift
	/usr/sadm/sysadm/bin/.chkuser -c "/usr/bin/backup $* :`cat $TMPFILE`:" 
else
echo "No users were MARKed for backup.  The MARK function"
echo "key must be used to select users to be backed up."
fi
rm -f $TMPFILE
