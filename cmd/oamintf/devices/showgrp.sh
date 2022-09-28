#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:devices/showgrp.sh	1.4.3.1"

################################################################################
#	Module Name: showgrp.sh
#	
#	Description: Determine how many devices and groups
#		     there are with given command attribute.
#		     There must be more than 15 devices and
#		     1 group or the group propmt is not
#		     displayed
#	Inputs:
#		$1 - device type
################################################################################
MINDEVICES=15
MINGROUPS=1

[ `getdev type=$1 | wc -l` -ge $MINDEVICES -a `getdgrp type=$1 | wc -l` -ge $MINGROUPS ] && exit 0 || exit 1

