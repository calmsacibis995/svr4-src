#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:devices/getdlst.sh	1.3.3.1"

################################################################################
#	Module Name: getdlst.sh
#	
#	Description: Displays to stdout a list of devices and descriptions
#		     that are the given type.
#		     The output will have the following format:
#
#		     diskette1:Floppy Drive 1
#		     diskette2:Floppy Drive 2
#		     diskette3:Floppy Drive 3
#
#	Inputs:
#		$1 - group
#		$2 - type
################################################################################
if [ "$1" ]
then
	list=`listdgrp $1`
	for x in `getdev type=$2 $list`
	do
		echo "$x\072\c"; devattr $x desc
	done
else
	for x in `getdev type=$2`
	do
		echo "$x\072\c"; devattr $x desc
	done
fi
