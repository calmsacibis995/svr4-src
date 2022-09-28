#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)mail:vacation.sh	1.1.3.1"
#
# Set up vacation facility to send a canned message back to 
# message originators alerting them to recipient's absense.
# Actualy installed by '/usr/bin/mail -F "|VACPROG -o %R [optional stuff]"
#
#
PATH=REAL_PATH
TMP=/tmp/notif$$
trap "rm -f $TMP" 0
VACPROG=/usr/lib/mail/vacation2
DEFMAILFILE=${HOME}/.mailfile
DEFLOGFILE=${HOME}/.maillog
DEFMSGFILE=USR_SHARE_LIB/mail/std_vac_msg
USAGE="\07USAGE: ${0} [-m mailfile] [-M canned_msg_file] [-l logfile] [-F failsafe] [-d]\07"
MAILFILE=${DEFMAILFILE}
LOGFILE=${DEFLOGFILE}
MSGFILE=${DEFMSGFILE}
SILENT=NO
DAILY=
FAILSAFE=
if [ ! -t 1 ]
then
	# stdout not a tty. Be as silent as possible.....
	SILENT=YES
fi

set -- `getopt m:M:l:F:d $*`
if [ ${?} -ne 0 ]
then
	case ${SILENT} in
	    NO ) echo ${USAGE} 1>&2 ;;
	esac
	exit 1
fi

for arg in ${*}
do
	case ${arg} in
	-m)	MAILFILE=${2}; shift 2;;
	-M)	MSGFILE=${2}; shift 2;;
	-l)	LOGFILE=${2}; shift 2;;
	-F)	FAILSAFE=$2; shift 2;;
	-d)	DAILY=.date; shift;;
	--)	shift; break;;
	esac
done

if [ -f "$LOGFILE" -a ! -w "$LOGFILE" ]
then
	echo "$0: Cannot write to $LOGFILE" 1>&2
	exit 1
fi
if [ -f "$LOGFILE" -a ! -r "$LOGFILE" ]
then
	echo "$0: Cannot read $LOGFILE" 1>&2
	exit 1
fi
if [ -f "$MAILFILE" -a ! -w "$MAILFILE" ]
then
	echo "$0: Cannot write to $MAILFILE" 1>&2
	exit 1
fi

# build up the command line
CMD="mail -F "
CMD="${CMD} \"|${VACPROG} -o %R"
if [ "x${MAILFILE}" != "x$DEFMAILFILE" ]
then
	CMD="${CMD} -m ${MAILFILE}"
fi
if [ "x${LOGFILE}" != "x$DEFLOGFILE" ]
then
	CMD="${CMD} -l ${LOGFILE}"
fi
if [ "x${MSGFILE}" != "x$DEFMSGFILE" ]
then
	CMD="${CMD} -M ${MSGFILE}"
fi
if [ -n "$FAILSAFE" ]
then
	CMD="${CMD} -F ${FAILSAFE}"
fi
if [ -n "$DAILY" ]
then
	CMD="${CMD} -d"
fi
CMD="${CMD}\" > $TMP 2>&1"
#
# Just to be safe
#
if  mail -F ""  > $TMP 2>&1
then	:
else
	cat $TMP
	exit 2
fi
if [ ! -f ${MAILFILE} ]
then
	> ${MAILFILE}
	chmod 600 ${MAILFILE}
fi
# Reset LOGFILE
> ${LOGFILE}

eval ${CMD}
if [ ${?} -ne 1 ]
then
	cat $TMP
	exit 2
fi
case ${SILENT} in
    NO )
	echo "${0}: Vacation notification installed"
	echo "${0}: New mail messages will go to '${MAILFILE}'"
	echo "${0}: Logging will go to '${LOGFILE}${DAILY}'"
	echo "${0}: '${MSGFILE}' will be used for the canned message"
	;;
esac
exit 0
