#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:usermgmt/uniq_gnam.sh	1.2.3.1"

################################################################################
#	Command Name: uniq_gnam
#
# 	This functions is used for checking if group name is unique.
#	 
#	Grep for the name passed in /etc/group, it must be at the
#	beginning of the line.  If the name is found return a 1
#	(false - since the name is already taken), return a 0 if
#	the name is not found (true - the name chosen is unique).
#
#	$1 - User input
#	$2 - Field default (only in groupmod)
################################################################################

test -z "$1" && exit 1

test "$1" = "$2" && exit 0

if grep "^$1:" /etc/group> /dev/null 2>&1
then
	exit 1
else
	exit 0
fi
