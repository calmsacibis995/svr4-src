#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)face:src/filecab/fileb/color_chk.sh	1.2"

color=$1
background=$2
text=$3

case $color in
	black | blue | cyan | green | magenta | red | white | yellow )
		if [ "$color" = "$background" ]
		then
			exit 2
		elif [ "$color" = "$text" ]
		then
			exit 3
		else
			exit 0
		fi;;
	*)
		exit 1;;
esac
