#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)filemgmt:bin/validfs.sh	1.1.2.1"

# Argument is not a directory

if [ ! -d ${1} ]
then
	exit 1
fi

# Mount point is busy

if [ ! -z "`/sbin/mount | sed -e 's/ .*//' | grep ${1}`" ]
then
	exit 2
fi

# Directory has files in it

if [ "`echo \`ls -la ${1} | wc -l\``" != "3" ]
then
	exit 3
fi
