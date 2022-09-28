#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)face:src/filecab/fileb/frame_chk.sh	1.1"

frame=$1
shift

if [ -z "$frame" ]
then
	echo 0; exit 0
fi

if [ -r "$frame" ]
then
	for other in "$@"
	do
		if [ "$frame" = "$other" ]
		then
			echo 2; exit 1
		fi
	done
else
	echo 1; exit 1
fi

echo 0
