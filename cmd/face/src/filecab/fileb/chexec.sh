#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)face:src/filecab/fileb/chexec.sh	1.3"

if [ -z "$1" ]
then
	exit 1
elif [ -x "$1" ]
then
	if [ -f "$1" ]
	then
		exit 0
	fi
else 	
	cmd="$1"
	not_found=`type "$cmd" | grep "not found"`
	if [ -z "$not_found" ]
	then
		exit 0
	fi
fi
exit 1
