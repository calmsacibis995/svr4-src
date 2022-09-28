#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)filemgmt:bin/devlst.sh	1.1.3.1"

################################################################################
#	Module Name: devlst.sh
#	Author: Mike Coufal - modified 'getdlst.sh' - Pati Porter
#	Date: April 1988
#	
#	Description: Displays to stdout a list of device drives and
#		     descriptions in the following format:
#
#		     ctape1:Cartridge Tape Drive 1
#		     diskette1:Floppy Drive 1
#		     diskette2:Floppy Drive 2
#
#	Inputs:
#		$1 - Device group
#		$2 - Command attribute1
################################################################################
if [ $1 ]
then
	for x in `listdgrp $1` 
	do
		if [ `devattr $x $2` ]
		then
			echo "$x\072\c"; devattr $x desc
		fi
	done
else
	for x in `getdev $2:*`
	do
		echo "$x\072\c"; devattr $x desc
	done
fi
