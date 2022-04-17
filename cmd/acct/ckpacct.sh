#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)acct:ckpacct.sh	1.9.2.2"
#       periodically check the size of /var/adm/pacct
#       if over $1 blocks (default: maxi 200, mini 500), execute 
#	turnacct switch

PATH=/usr/lib/acct:/usr/bin:/usr/sbin
trap "rm -f /var/adm/cklock*; exit 0" 0 1 2 3 9 15
export PATH

if [ -f uts -a uts ]
then
	_max=${1-200}
else
	_max=${1-500}
fi
_MIN_BLKS=500
cd /var/adm

#	set up lock files to prevent simultaneous checking

cp /dev/null cklock
chmod 400 cklock
ln cklock cklock1 > /dev/null 2>&1
if test $? -ne 0 ; then exit 1; fi

#	Find a non-existent file in /tmp with prefix accttmp and create it.
#	Accounting may be momentarily switched on to it.

TDIR=/tmp
IND=1

while test -r $TDIR/accttmp${IND}
do
	IND=`expr ${IND} + 1`
done
TEMPFILE=${TDIR}/accttmp${IND}

echo "" > ${TEMPFILE} #	Temporary file is created.

#	check to see if accounting enabled. If "accton $TEMPFILE"
#	exits 1, then accounting was enabled.


accton ${TEMPFILE} 2> /dev/null	#	don't print potential error msg
			 	#	to stderr
ACCTSTAT=$?
if test ${ACCTSTAT} -eq 0
then	     
	 accton  #	accting was disabled. Disable it again
fi
rm -f ${TEMPFILE}

#	If there are less than $_MIN_BLKS free blocks left on the /var
#	file system, turn off the accounting (unless things improve
#	the accounting wouldn't run anyway).  If something has
#	returned the file system space, restart accounting.  This
#	feature relies on the fact that ckpacct is kicked off by the
#	cron at least once per hour.

_dev=`devnm /var | awk '{print $1}' | sed -e "s;/dev/;;"`
_blocks=`df /dev/$_dev | sed "s/.*:  *\([0-9][0-9]*\) blocks.*/\1/"`

if [ "$_blocks" -lt $_MIN_BLKS   -a  -f /tmp/acctoff ];then
	echo "ckpacct: /var still low on space ($_blocks blks); \c"
	echo "acctg still off"
	( echo "ckpacct: /var still low on space ($_blocks blks); \c"
	echo "acctg still off" ) | mail root adm
	exit 1
elif [ "$_blocks" -lt $_MIN_BLKS ];then
	if test ${ACCTSTAT} -ne 0; then
		echo "ckpacct: /var too low on space ($_blocks blks); \c"
		echo "turning acctg off"
		( echo "ckpacct: /var too low on space ($_blocks blks); \c"
		echo "turning acctg off" ) | mail root adm
		nulladm /tmp/acctoff
		turnacct off
		exit 1
	fi
elif [ -f /tmp/acctoff ];then
	echo "ckpacct: /var free space restored; turning acctg on"
	echo "ckpacct: /var free space restored; turning acctg on" | \
		mail root adm
	rm /tmp/acctoff
	turnacct on
fi

_cursize="`du -s pacct | sed 's/	.*//'`"
if [ "${_max}" -lt "${_cursize}" ]; then
	turnacct switch
fi
