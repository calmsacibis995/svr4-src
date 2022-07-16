#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

:
#
#	INTEL CORPORATION PROPRIETARY INFORMATION
#
#	This software is supplied under the terms of a license agreement
#	or nondisclosure agreement with Intel Corporation and may not be
#	copied nor disclosed except in accordance with the terms of that
#	agreement.
#
#	Copyright 1988  Intel Corporation
#
#ident	"@(#)mbus:cmd/enet/ina961_530.sh	1.3.3.1"
#

#
# ina961:	Download ina961 software to 186/530 board -or- reset board
#
#		usage: ina961 [ start | stop ]
#
#	This script is linked to the /etc/rc2.d directories allowing
#	automatic startup and shutdown of the Multibus II transport layer as
#	the run state of UNIX changes [see modrc(1)].
#
state=$1
set `who -r`
p7=$7	# current run level
p8=$8	# count of times at this run level
p9=$9	# previous run level
#
#
boardlist="186/530 MIX386/560"
boardtype=

case $state in

'start')
	#
	# Only do this if previous run state was "0" or "S"
	#
	if [ "$p9" = "0" -o "$p9" = "S" ] ; then
          /etc/conf/bin/idcheck -p clone 
          if [ $? -gt 0 -a $? -lt 100 ]; then
            /etc/conf/bin/idcheck -p mb2stai
            if [ $? -gt 0 -a $? -lt 100 ]; then
               if [ -f /usr/sbin/lckld ] ; then
                 if [ -f /etc/ina961.36 ] ; then
                    for slotnum in  0  1  2  3  4  5  6  7  8  9 \
                                   10 11 12 13 14 15 16 17 18 19
                    do
		      for boardtype in $boardlist
		      do
                       if [ "`/usr/sbin/icsrd -s $slotnum 2 10`" = "$boardtype" > /dev/null 2>&1 ] ; then
                          /usr/sbin/ina $slotnum $boardtype
			  break
                       fi
		      done
                    done
                 else
                    exit
                 fi
               else
                 exit
               fi
            else
               exit
            fi
          else
            exit
          fi
	fi
     ;;
'stop')
# Can not shutdown the iSBC 186/530 Comm. Board, because in a system
# with more than one processor the processor shutting down may not
# be the processor controlling the iSBC 186/530
#     for slotnum in  0  1  2  3  4  5  6  7  8  9 \
#                    10 11 12 13 14 15 16 17 18 19
#     do
#        if [ "`/etc/icsrd -s $slotnum 2 10`" = "186/530" > /dev/null 2>&1 ]
#        then
#            /usr/ibin/reset $slotnum
#        fi
#     done
      ;;
esac
