#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)mail:vacation2.sh	1.1.3.1"
#
# Second half of vacation(1).
# When new message arrives, save it in MAILFILE and check any prior messages
# from the same ORIGINATOR have been received. If so, exit. If not, record
# ORIGINATOR in LOGFILE and send canned message from MSGFILE to ORIGINATOR.
#
PATH=REAL_PATH
USAGE="\07USAGE: ${0} -ooriginator [-m mailfile] [-M canned_msg_file] [-l logfile] [-F failsafe-path] [-d]"
ORIGINATOR=
MAILFILE=${HOME}/.mailfile
LOGFILE=${HOME}/.maillog
MSGFILE=USR_SHARE_LIB/mail/std_vac_msg
SILENT=NO
FAILSAFE=
DAILY=NO
TMP=/tmp/.vac.$$

if [ ! -t 1 ]
then
	# stdout not a tty. Be as silent as possible.....
	SILENT=YES
fi

set -- `getopt o:m:M:l:F:d $*`
if [ ${?} -ne 0 ]
then
	case "$SILENT" in
	    NO ) echo ${USAGE} 1>&2 ;;
	esac
	exit 1
fi

for arg in ${*}
do
	case ${arg} in
	-o)	ORIGINATOR=$2; shift 2;;
	-m)	MAILFILE=$2; shift 2;;
	-M)	MSGFILE=$2; shift 2;;
	-l)	LOGFILE=$2; shift 2;;
	-F)	FAILSAFE=$2; shift 2;;
	-d)	DAILY=YES; shift;;
	--)	shift; break;;
	esac
done
if [ -z "${ORIGINATOR}" ]
then
	case "$SILENT" in
	    NO ) echo ${USAGE} 1>&2 ;;
	esac
	exit 1
fi

case $DAILY in
    YES ) MAILFILE=$MAILFILE.`date +%m%d` ;;
esac

# append to the saved-mail file
trap 'rm -f $TMP' 0 1 2 3 15
if tee $TMP >> ${MAILFILE}
then :
else
	if [ -n "$FAILSAFE" ]
	then rmail "$FAILSAFE" < $TMP; exit 0
	else exit 1
	fi
fi

# notify the originator
if fgrep -x ${ORIGINATOR} ${LOGFILE} > /dev/null 2>&1
then
	:
else
	echo ${ORIGINATOR} >> ${LOGFILE} 2>/dev/null
	( cat ${MSGFILE} | mail ${ORIGINATOR} ) &
fi
exit 0
