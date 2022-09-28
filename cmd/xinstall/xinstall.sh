#! /sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1987, 1988 Microsoft Corporation
#	  All Rights Reserved

#	This Module contains Proprietary Information of Microsoft
#	Corporation and should be treated as Confidential.

#ident	"@(#)xinstall:xinstall.sh	1.1.1.1"

#	@(#) xinstall.sh 1.3.2.3 90/05/21 xinstall:xinstall.sh
#
#	Xenix V Installation 
#	====================
#	This script untars the distribution diskettes, and sets file
#	permissions.
#
#	The strings $BASE, $SOFT and $TEXT list the files from the
#	respective packages that are to be preserved from the previous
#	release.  New ones are moved to *.X5 .
#
#	The strings $CBASE, ... list the files to be backed up from the
#	previous releases; ie. old ones are move to *.X3.  The strings
#	$DBASE, ... list the links to be broken.

PATH=/sbin:/usr/bin:/usr/sbin

trap   'echo;						\
	echo "";					\
	echo "$0: INTERRUPTED - type $0 to restart";	\
	exit 2'						\
1 2 3 15

DEVICE=/dev/rfd0

case $# in

  0)	name="base"
	;;

  1)	if [ -c "$1" -o -b "$1" ]
	then DEVICE="$1"
	else name="$1"
	fi
	;;

  2)	if [ -c "$1" -o -b "$1" ]
	then DEVICE="$1"
	     name="$2"
	else if [ -c "$2" -o -b "$2" ]
	     then DEVICE=$2
	          name="$1"
	     else echo "$0: Illegal device name $1 or $2"
		  echo "Usage: $0 [device] [package name]"
		  exit 1
	     fi
	fi
	;;
  *)	echo "Usage: $0 [device] [package name]"
	exit 1
	;;
esac


case "$name" in

base)	pack="BASE"
	;;

soft)	pack="SOFTWARE DEVELOPMENT"
	;;

soft286) pack="286 SOFTWARE DEVELOPMENT"
	;;

text)	pack="TEXT PROCESSING"
	;;

update)	pack="UPDATE"
	;;

*)	echo "Un-recognized package $name"
	echo "Usage: $0 [device] [package name]"
	exit 1
	;;
esac

#	Untar the Distribution
#	======================

clear
echo
echo "	Installation of XENIX System V/386"
echo
echo "	Installing the $pack Package"
echo
echo "Install the XENIX System V/386 distribution diskettes in order"
echo "starting with number 1. If you are installing more than one XENIX"
echo "option, you must install the base package first."
echo
echo "Follow the instructions on your screen as the installation"
echo 'proceeds.  After installing the last diskette, answer "n" to'
echo "the prompt and press ENTER."
echo "\n\n\n"
echo "Press ENTER to continue: \c"
read reply

clear
echo "Insert each diskette after the drive light goes out."
echo 'Answer "y" to the prompt and press ENTER.'
echo
echo 'After installing the last diskette, answer "n" to the prompt'
echo "and press ENTER."

cd /
modifier="First"

while	:
do
	echo
	echo "$modifier diskette? [y,n] \c"
	read reply rest

	case $reply in

		y|Y)	echo "\nExtracting files from diskette ....."
			tar xvfb ${DEVICE} 20
			modifier="Next"
			sync
			;;

		n|N)	break
			;;

		*)	echo  'please answer with "y" or "n".'
			;;
	esac
done

#	Set File Permissions
#	====================

trap   'echo;						\
	echo "";					\
	echo "$0: INTERRUPTED - type $0 to restart";	\
	exit 2'						\
1 2 3 15

cd /
if	[ -f /etc/$name.perms ]
then
	clear
	echo "Installation program setting file permissions; please stand by:"
	/etc/fixperm -S -dB -c /etc/$name.perms >/tmp/$name.log 2>&1
	/etc/fixperm -S -dS -c /etc/$name.perms >>/tmp/$name.log 2>&1
	/etc/fixperm -S -dT -c /etc/$name.perms >>/tmp/$name.log 2>&1
	sync
elif [ -f /etc/perms/$name ]
then
	clear
	echo "Installation program setting file permissions; please stand by:"
	/etc/fixperm -S -dB -c /etc/$name.perms >/tmp/$name.log 2>&1
	/etc/fixperm -S -dS -c /etc/$name.perms >>/tmp/$name.log 2>&1
	/etc/fixperm -S -dT -c /etc/$name.perms >>/tmp/$name.log 2>&1
	sync
fi

cd /

echo "\n"
echo "XENIX System V/386 $pack PACKAGE INSTALLATION COMPLETED"
echo
echo 'Now return to the installation section of "Starting XENIX."\n'

exit 0


