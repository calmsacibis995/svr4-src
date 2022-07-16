#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1987  Intel Corporation
#	All Rights Rreserved
#
#	INTEL CORPORATION PROPRIETARY INFORMATION
#
#	This software is supplied to AT & T under the terms of a license 
#	agreement with Intel Corporation and may not be copied nor 
#	disclosed except in accordance with the terms of that agreement.

#ident	"@(#)mbus:proto/i386/mbus/cmd/main.sh	1.3.2.2"
#
#
CPU="386"
VERS="1"
MENU="Tape Distribution "
menu=$MENU
ROOT=${ROOT:-"/mnt"}
INSTDIR=${INSTDIR:-/opt}
SAVE_LOC=$INSTDIR/unix/save.d
TERM=${TERM:-ansi}
TERMINFO=$ROOT/usr/share/lib/terminfo
DSK=${DSK:-`cpunum -t`}
PARAMS_SET=${PARAMS_SET:-0}
DEF_DSK_SET=${DEF_DSK_SET:-0} 
OFS="$IFS"
PATH=\
/sbin:/bin:/usr/sbin:/usr/bin:/etc:/opt/unix::/mnt/sbin:/mnt/etc:/mnt/usr/sbin:\
/mnt/usr/bin:/mnt/bin:/floppy
#
# get current enviornment if any
set -a
[ -f /tmp/env ] && {
	. /tmp/env
	rm /tmp/env
}
#
# variable stuff
#
SWAPON=${SWAPON:-0}
DEBUG=${DEBUG:-"0"}
#
export PATH TERM TERMINFO ROOT CPU REL VERS
export SAVE_LOC SWAPON MENU DEBUG  DSK PARAMS_SET DEF_DSK_SET
export heads cyls secs gran intrl
#
# get support functions
#
#
# initial setup
umask 000  
ulimit 100000
trap '' 1 3 9 15
. /opt/unix/lib.func
. /opt/unix/main.menu
[ -f /tmp/mounts ]     || >/tmp/mounts
>/etc/mnttab
#
#
REL="V4.0"
stty intr '^c' erase '^?'  -tabs echoe
#
date
Pause
echo "Main Installation Menu \n`date` " 
#
# and go to Main menu

while :
do
	Main_menu
	Select 
	#
	# MAIN menues:
	#
	#	1-Install 
	#	3-Recovery 
	#
	case $entry in 
		#
	1|[sS]*)
		case $DEBUG in
		    0) /opt/unix/install.sh ;;
		    1) /opt/unix/install.sh 2>&1 |tee -a /floppy/log.dbg;;
		 esac
		 ;;
	2|[mM]*) 
		Do_def_dsk
		echo "and exiting to shell."
		echo "\n\nType exit to return"
		Do_swap
		#Do_allmount 
		/bin/sh  -i
		#Do_umount
		Do_unswap 
		;;
	3|[fFlL]*)
		case $DEBUG in
		    0) /opt/unix/format.sh;;
		    1) /opt/unix/format.sh 2>&1 |tee -a /floppy/log.dbg;;
		 esac
		 ;;
		#
	######################
	## standard options ##
	######################
		#
	[rR]*)	continue 
		;;
	[qQ]*)	Quit 
		;;
	   !) /bin/sh -i
		;;
	   !*) tmp="$*"
		set $entry
		shift
		eval "$*"
		[ -n "$tmp" ] && set $tmp
		Pause
		;;
	 don) set -x
		;;
	doff) set +x
		  DEBUG=0
		;;
	dskin)	Do_mount -n /dev/dsk/f0t /floppy  s5 >/dev/null 2>&1
			[ $? -ne 0 ] && {
				mkfs /dev/dsk/f0t 720 1 8
				Do_mount /dev/dsk/f0t /floppy  s5
			}
			[ -f /floppy/log.dbg ] && >/floppy/log.dbg
			DEBUG=1
			Pause
		;;
        *)	echo "Invalid Response \c"
		Pause
		continue
		;;
	esac
done
echo "Exiting from Main.sh"
Env
Pause
