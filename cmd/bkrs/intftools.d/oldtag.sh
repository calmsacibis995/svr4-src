#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)bkrs:intftools.d/oldtag.sh	1.2.3.1"

# Validate that tag ($1) exists in table ($2).
# Note that a null tag is not valid.

if [ -s "$2" ]
then
	if [ -z "$1" ]
	then
		exit 1
	else
# tag_exists returns true if tag exists
		tag_exists $1 $2
		exit $?
	fi
else
	exit 2
fi
