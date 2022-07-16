#! /sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# Copyright (c) 1988, Sun Microsystems, Inc.
# All Rights Reserved.

#ident	"@(#)ucbhostname:hostname.sh	1.2.2.1"


if [ $# -eq 0 ]; then
	uname -n
elif [ $# -eq 1 ]; then
	uname -S $1
     else
	echo Usage: hostname [name]
	exit 1
fi
