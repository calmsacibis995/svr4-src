#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:usermgmt/addgrp.sh	1.2.3.1"

################################################################################
#	Command Name: addgrp
#
#	Description: This scripts does 3 things:
#			1) adds group to the system
#		     	2) changes primary group for specified logins
#			3) adds supplementary group status to specified logins.
#
# 	Inputs:		$1 - Group name
# 			$2 - group ID
# 			$3 - primary grou(s)
# 			$4 - supplementary grou(s)
################################################################################

NOADD=1
BADPRIM=2
BADSUP=3

# add group to system
groupadd -g $2 $1 2> /tmp/gadderr || exit $NOADD

# change primary group for specified logins
if [ $3 ]
then
	for x in `echo $3 | sed 's/,/ /g'`
	do
		usermod -g "$1" "$x" > /dev/null 2>> /tmp/gadderr
	done || exit $BADPRIM
fi

# add supplementary group members
if [ $4 ]
then
	addgrpmem -g $1 `echo $4 | sed 's/,/ /g'` > /dev/null 2>&1 || $BADSUP
fi
