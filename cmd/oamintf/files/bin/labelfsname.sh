#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:files/bin/labelfsname.sh	1.2.1.1"
#	given the output of /etc/labelit, return the file system name and
#	volume label as shell variable assignments.

#!	chmod +x ${file}

if [ $# -eq 0 ]
then
	echo >&2 "Usage:  $0 /etc/labelit-output"
	exit 1
fi

echo $*  |
	sed -n 's/Current fsname: \(.*\), Current volname: \(.*\), Blocks: .*/fsname=\1 label=\2/p'
