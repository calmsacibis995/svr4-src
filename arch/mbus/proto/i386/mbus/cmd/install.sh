#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

:
#	Copyright (c) 1987  Intel Corporation
#	All Rights Rreserved
#
#	INTEL CORPORATION PROPRIETARY INFORMATION
#
#	This software is supplied to AT & T under the terms of a license 
#	agreement with Intel Corporation and may not be copied nor 
#	disclosed except in accordance with the terms of that agreement.

#ident	"@(#)mbus:proto/i386/mbus/cmd/install.sh	1.3.1.1"

MENU="Software Installation"
menu=$MENU
set -a
[ -f /tmp/env ] && {
	. /tmp/env
	rm /tmp/env
}
#
export PATH TERM TERMINFO ROOT CPU REL VERS INSTDIR 
export SAVE_LOC SWAPON MENU DEBUG  DSK DEF_DSK_SET PARAMS_SET
# get install menu's
. /opt/unix/install.menu
#
# main loop
#
Clear

Do_def_dsk

while :
do
STAT=0
Install_menu
Select 
#
# MAIN options:
#
#	1-Format Install 
#	2-Over Install
#
set -a
DSK=${DSK:-"0"}			# disk to work on (for /dev/dsk/${DSK}s1)	
Clear

case $entry in 
	1|[nN]*) Install_init_menu
		echo "You have chosen to ERASE all data on \c"
		echo "HARD DISK $DSK\n"
		Ask "\nDo you wish to continue <y/[n]>: \c"
		case $answer in
			[yY]*)	: ;;
			''|*)	break;;
		esac
		#
		# now go do it
		#
		Do_parts $DSK					||Err Do_parts
		Do_dsetup $DSK					||Err Do_dsetup
		Do_disksetup $DSK				||Err disksetup 
		Do_cpio  -R -S     	     			||Err Do_cpio 
		Do_setup  init 		  			||Err Do_setup 
		Do_umount 	   				||Err Do_umount 
		echo "\nUnix System V/386 installation is now complete \n"
		Pause
		;;
       2|[uU]*) Install_over_menu
		sflags=""
		rflags=""
		Pause
		echo "Upgrading Software on Disk $DSK "
		Ask "Restore saved files at end of installation <[y]/n> ? \c" 
		case $answer in
			[nN]*)		rflags="-exit" #exit 
					;;
			*)	rflags=""
				Ask "\nDo You wish to be selective <y/[n]> ? \c"
				case $answer in
					[yY]*)	rflags="$rflags -e";;
				esac
				;;
		esac
		#
		# and go do it
		#
		Do_allmount   		                 	||Err Do_mount of all 
		Do_swap                		         	||Err Do_swap 
		Do_cpio $rflags $sflags -r $ROOT        ||Err Do_cpio 
		Do_setup update	  						||Err Do_setup 
		Do_umount 	    		         		||Err Do_umount 
		Do_unswap          		         		||Err Do_unswap 
		echo "\nUnix System V/386 system upgrade is now complete\n"
		Pause
		;;
	######################
	## standard options ##
	######################
		#
	[rR]*)	break
		;;
	[qQ]*)	Quit 
		;;
	   !)	/bin/sh -i
		;;
	   !*)	tmp="$*"
			set $entry
			shift
			eval "$*"
			[ -n "$tmp" ] && set $tmp
			Pause
		;;
	 don)	set -x
			DEBUG=1
		;;
	doff)	set +x
			DEBUG=0
		;;
        *)	echo "Invalid Response \c"
		Pause
		continue
		;;
	esac
done
STAT=0
Env
