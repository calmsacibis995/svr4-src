#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)bkrs:intftools.d/errnewtag.sh	1.2.3.1"

# Invoked with tag ($1) and table ($2) and reports either that a null tag
# is invalid or that tag already exists in table.

if [ "$1" ]
then
	echo "Tag already exists in table $2 or has blanks in it."
else
	echo "The tag value cannot be null."
fi
