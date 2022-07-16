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

#ident	"@(#)mbus:proto/i386/mbus/cmd/format.sh	1.3"

MENU="Disk Format"
menu=$MENU
set -a
[ -f /tmp/env ] && {
	. /tmp/env
	rm /tmp/env
}
#
export PATH TERM TERMINFO ROOT CPU REL INSTDIR 
export SAVE_LOC SWAPON MENU DEBUG  DSK
# get install menu's
. /opt/unix/format.menu
#
# main loop
#
while :
do
STAT=0
Install_menu
Select 
#
# MAIN options:
#
#	1-Set disk params
#	3-Edit MDL
#	2-Format Install 
#	4-Install new boot strap
#
set -a
case $entry in 
	1|[nN]*) 
		Clear
		Do_def_dsk
		Do_dsk_args
		Pause
		;;
	2|[nN]*) 
		Clear
		if [ $DEF_DSK_SET -eq 0 ];then	
			Do_def_dsk
		fi
		if [ $PARAMS_SET -eq 0 ];then	
			Do_dsk_args
		fi
		hdformat -i $intrl -c $cyls -f $heads -d $gran -s $secs \
			 /dev/rdsk/${DSK}s0  
		Pause
		;;
       3|[uU]*) 
		Clear
		if [ $DEF_DSK_SET -eq 0 ];then	
			Do_def_dsk
		fi
		if [ $PARAMS_SET -eq 0 ];then	
			Do_dsk_args
		fi
		disksetup -b /etc/dsboot -B /dev/rdsk/${DSK}s0
		Pause
		;;
       4|[bB]*) 
		Clear
		if [ $DEF_DSK_SET -eq 0 ];then	
			Do_def_dsk
		fi
		if [ $PARAMS_SET -eq 0 ];then	
			Do_dsk_args
		fi
		mdl -A -c $cyls -f $heads -d $gran -s $secs \
			/dev/rdsk/${DSK}s0
		echo "Now select format action to initialze the disk"
		Pause
		;;
		5)
		Clear
		if [ $DEF_DSK_SET -eq 0 ];then	
			Do_def_dsk
		fi
		if [ $PARAMS_SET -eq 0 ];then	
			Do_dsk_args
		fi
		mdl  -c $cyls -f $heads -d $gran -s $secs \
			/dev/rdsk/${DSK}s0
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
	:
done
STAT=0
Env
