#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:machinemgmt/chgstate.sh	1.2.5.1"

################################################################################
#	Module Name: chgstate.sh
#	Inputs:
#		$1 - grace period
#		$2 - init state
################################################################################

# Run the process in background, give the interface 5 seconds to exit,
# send all output to /dev/null so as not to confuse fmli

nohup sh <<! > /dev/null 2>&1 &
sleep 5
cd /
shutdown -y -g"$1" -i$2
!
