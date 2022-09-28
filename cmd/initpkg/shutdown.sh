#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)initpkg:./shutdown.sh	1.8.14.1"

#	Sequence performed to change the init stat of a machine.

#	This procedure checks to see if you are permitted and allows an
#	interactive shutdown.  The actual change of state, killing of
#	processes and such are performed by the new init state, say 0,
#	and its /sbin/rc0.

#	Usage:  shutdown [ -y ] [ -g<grace-period> ] [ -i<init-state> ]

if [ `pwd` != / ]
then
	echo "$0:  You must be in the / directory to run /sbin/shutdown."
	exit 1
fi

# Make sure /usr is mounted
if [ ! -d /usr/bin ]
then
	echo "$0:  /usr is not mounted.  Mount /usr or use init to shutdown."
	exit 1
fi

#	Check the user id
if [ -x /usr/bin/id ]
then
	eval `id  |  /usr/bin/sed 's/[^a-z0-9=].*//'`
	if [ "${uid:=0}" -ne 0 ]
	then
	        echo "$0:  Only root can run /sbin/shutdown."
		exit 2
	fi
else
	echo "$0:  can't check user id."
	exit 2
fi

grace=60
askconfirmation=yes

if i386
then
	initstate=0
else
	initstate=s
fi

while [ $# -gt 0 ]
do
	case $1 in
	-g[0-9]* )
		grace=`/usr/bin/expr "$1" : '-g\(.*\)'`
		;;
	-i[Ss0156] )
		initstate=`/usr/bin/expr "$1" : '-i\(.*\)'`
		;;
	-i[234] )
		initstate=`/usr/bin/expr "$1" : '-i\(.*\)'`
		echo "$0:  Initstate $initstate is not for system shutdown."
		exit 1
		;;
	-y )
		askconfirmation=
		;;
	-* )
		echo "$0:  Illegal flag argument '$1'"
		exit 1
		;;
	* )
		echo "Usage:  $0 [ -y ] [ -g<grace> ] [ -i<initstate> ]"
		exit 1
	esac
	shift
done

if [ -x /usr/alarm/bin/event ]
then
	/usr/alarm/bin/event -c gen -e shutdown -- -t $grace
fi

if [ -z "${TZ}"  -a  -r /etc/TIMEZONE ]
then
	. /etc/TIMEZONE
fi

echo '\nShutdown started.    \c'
/usr/bin/date
echo

/sbin/sync
cd /

trap "exit 1"  1 2 15

a="`/sbin/who  |  /usr/bin/wc -l`"
if [ ${a} -gt 1  -a  ${grace} -gt 0 ]
then
	/usr/sbin/wall<<-!
		The system will be shut down in ${grace} seconds.
		Please log off now.

	!
	/usr/bin/sleep ${grace}
fi

/usr/sbin/wall <<-!
	THE SYSTEM IS BEING SHUT DOWN NOW ! ! !
	Log off now or risk your files being damaged.

!
/usr/bin/sleep ${grace}

if [ ${askconfirmation} ]
then
	echo "Do you want to continue? (y or n):   \c"
	read b
else
	b=y
fi
if [ "$b" != "y" ]
then
	/usr/sbin/wall <<-!
		False Alarm:  The system will not be brought down.
	!
	echo 'Shut down aborted.'
	exit 1
fi
case "${initstate}" in
s | S )
	. /sbin/rc0
esac

echo "Changing to init state $initstate - please wait"
/sbin/init ${initstate}
