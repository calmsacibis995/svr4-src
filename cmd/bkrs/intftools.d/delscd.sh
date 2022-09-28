#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)bkrs:intftools.d/delscd.sh	1.3.3.1"
# Script to remove selected lines from file containing crontab lines.
# Script expects name of file as its first argument.  The second argument
# is the tag for the schedule lines desired (bksched or bkmsg).  The remaining
# arguments are the numbers of the lines to be deleted.  This script
# creates another temporary file and removes both after the crontab
# has been modified.

#set -x
TFILE1=$1
SCHED=$2
LINES=$3
SEDFILE=/tmp/bksed$$
TFILE2=/tmp/bktmp2$$
CFILE=/tmp/bktmpc$$

# substitute spaces for commas between line number entries
LINES=`echo $LINES | sed -e "s/,/ /g"`

# if temp file with crontab lines doesn't exist, quit
if [ ! -s $TFILE1 ]
then
	exit 1
fi

# if the sed script file already exists, remove it
if [ -s $SEDFILE ]
then
	rm $SEDFILE
fi

for i in $LINES
do
	echo ${i}d >>$SEDFILE
done

# produce file of backup schedule lines with appropriate ones deleted
sed -f $SEDFILE $TFILE1 >$TFILE2
if [ $? -ne 0 ]
then
	rm -f $TFILE1 $TFILE2 $SEDFILE
	exit 1
fi

# create a new crontab consisting of the old crontab lines that are
# not backup schedule lines with the new backup schedule lines appended.
if [ -s $TFILE2 ]
then
	crontab -l | grep -v \#${SCHED}\# | cat - $TFILE2 >$CFILE
else
	crontab -l | grep -v \#${SCHED}\# >$CFILE
fi

crontab <$CFILE 2>/dev/null
RC=$?

# clean up temp files
rm -f $TFILE1 $TFILE2 $CFILE $SEDFILE
exit $RC
