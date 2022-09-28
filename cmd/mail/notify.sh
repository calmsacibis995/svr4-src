#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)mail:notify.sh	1.1.3.1"
#
# Set up asynchronous notification of new incoming mail
# by doing a 'mail -F "|NOTIFYPROG -m mailfile -o %R -s %S"
#
# notify -n ==> turn it off.
# notify -y ==> turn it on. use default of $HOME/.mailfile for new mailbox.
# notify -m mailfile ==> turn it on. use mailfile for new mailbox.
# notify (no args) ==> Just check if currently activated and report findings.
#

PATH=REAL_PATH
TMP=/tmp/notif$$
trap "rm -f $TMP" 0
NOTIFYPROG=/usr/lib/mail/notify2
USAGE="\07USAGE: ${0} [[-]y/[-]n] [-m mailfile]\07"
MSG1="${0}: Mail notification"
MSG2="${0}: Notification cannot be installed unless "
NEWMAILFILE=${HOME}/.mailfile
YFLAG=0;
NFLAG=0;
SILENT=NO
if [ ! -t 1 ]; then
	# stdout not a tty. Be as silent as possible.....
	SILENT=YES
fi

set -- `getopt ynm: $*`
if [ ${?} -ne 0 ]
then
	if [ ${SILENT} = NO ]
	then
		echo ${USAGE} 1>&2
	fi
	exit 2
fi
for arg in ${*}
do
	case ${arg} in
	-n)	NFLAG=1; shift;;
	-y)	YFLAG=1; shift;;
	-m)	NEWMAILFILE=${2}; YFLAG=1; shift 2;;
	--)	shift; break;;
	esac
done
#
# If any args left, assume they 'forgot' the leading dash...:-)
#
if [ ${#} -gt 0 ]
then
	case "$1" in
	    [Yy]* ) YFLAG=1 ;;
	    [Nn]* ) NFLAG=1 ;;
	    * )	    
		if [ ${SILENT} = NO ]
		then
			echo ${USAGE} 1>&2
		fi
		exit 2
	esac
fi

# check file name
case "$NEWMAILFILE" in
    /* ) ;;
    * ) echo "$0: must use full path for $NEWMAILFILE" 1>&2; exit 1;;
esac

# Get the name of the person from their id.
# id returns: uid=unum(uname) gid=gnum(gname) ...
set -- `id`
case "$1" in
    # If running as root, allow $LOGNAME to override.
    uid=0'('*')' )	MYNAME=${LOGNAME:-`expr "$1" : ".*(\(.*\))"`} ;;
    # root is not in /etc/passwd? This should never happen.
    uid=0 )		MYNAME=${LOGNAME:-root} ;;
    # Extract the name.
    uid=*'('*')' )	MYNAME=`expr "$1" : ".*(\(.*\))"` ;;
    # The user is no longer in the database? Exit if $LOGNAME isn't there.
    * )			MYNAME=${LOGNAME:?No user name?} ;;
esac

if [ ${NFLAG} -eq 1 -o ${YFLAG} -eq 0 ]
then
	if [ ! -f "VAR_MAIL/${MYNAME}" ]
	then
		if [ ${SILENT} = NO ]
		then
			echo "${MSG1} not active"
		fi
		exit 0
	fi
	read rest < VAR_MAIL/${MYNAME}
	MFILE=`expr "x${rest}" : \
		"xForward to |${NOTIFYPROG} -m \(.*\) -o %R -s %S"`
	if [ "x${MFILE}" = x ]
	then
		if [ ${SILENT} = NO ]
		then
			echo "${MSG1} not active"
		fi
		exit 0
	fi
	if [ ${NFLAG} -eq 1 ]
	then
		#
		# Turn notification facility off
		#
		MAIL=VAR_MAIL/${MYNAME} mail -F "" > /dev/null 2>&1
		if [ ${SILENT} = NO ]
		then
			echo "${MSG1} deactivated. \c"
			echo "New mail will go into VAR_MAIL/${MYNAME}"
			echo "${0}: Old messages may be in '${MFILE}'"
			echo "${0}: Be sure to redefine \${MAIL} appropriately"
		fi
		exit 0
	fi
	#
	# Just report finding
	#
	if [ ${SILENT} = NO ]
	then
		echo "${MSG1} active."
		echo "${0}: New mail messages will go to '${MFILE}'"
	fi
	exit 0
fi
#
# Set up for notification. If already there and 'mailfile' is the same,
# do nothing. If different, prompt user for confirmation before changing
#
if [ -f "VAR_MAIL/${MYNAME}" -a -s "VAR_MAIL/${MYNAME}" ]
then
	read rest < VAR_MAIL/${MYNAME}
	MFILE=`expr "x${rest}" : \
		"xForward to |${NOTIFYPROG} -m \(.*\) -o%R %S"`
	if [ "x${MFILE}" = x ]
	then
		if [ ${SILENT} = NO ]
		then
			echo "${MSG2} VAR_MAIL/${MYNAME} is empty"
		fi
		exit 0
	fi
	if [ "${MFILE}" != "${NEWMAILFILE}" ]
	then
		if [ ${SILENT} = NO ]
		then
		    echo "${0}: Notification already installed."
		    echo "${0}: Current alternate mailbox is '${MFILE}'"
		    echo "${0}: Do you want to change it to '${NEWMAILFILE}' \c"
		    echo "(y or n): \c"
		    read ANS
		    case "$ANS" in
		        [Nn]* )
			    echo "${0}: No change"
			    exit 0
			    ;;
		    esac
		else
		    # Assume no change, since we can't ask the question...
		    exit 0
		fi
	fi
fi

# check the new mail file
if [ ! -f ${NEWMAILFILE} ]
then
	if  : > ${NEWMAILFILE} &&
	    chmod 600 ${NEWMAILFILE}
	then :
	else
		echo "$0: cannot properly access ${NEWMAILFILE}" 1>&2
		exit 1
	fi
elif [ ! -w ${NEWMAILFILE} ]
then
	echo "$0: ${NEWMAILFILE} is not writable!" 1>&2
	exit 1
fi

#
# Just to be safe
#
if	MAIL=VAR_MAIL/${MYNAME} mail -F "" > /dev/null 2>&1
then	:
else
	echo "$0: Cannot install notification" 1>&2
	exit 2
fi
if  MAIL=VAR_MAIL/${MYNAME} \
	mail -F "|${NOTIFYPROG} -m ${NEWMAILFILE} -o %R -s %S" > /dev/null 2>&1
then	:
else
	echo "$0: Cannot install notification" 1>&2
	exit 2
fi
if [ ${SILENT} = NO ]
then
	echo "${0}: Asynchronous 'new mail' notification installed"
	echo "${0}: New mail messages will go to '${NEWMAILFILE}'"
	echo "${0}: Be sure to re-define \${MAIL} appropriately"
fi
exit 0
