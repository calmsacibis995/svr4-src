#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)face:src/filecab/fileb/chkterm.sh	1.1"

if [ $# = 0 ]
then
	echo Usage: chkterm term-identifier
	exit 2
fi
terminfo=${TERMINFO:-/usr/lib/terminfo}
term=${1}
dir=`echo ${term} | cut -c1`
if [ -f  "${terminfo}/${dir}/${term}" ]
then
	exit 0
else
	exit 1
fi
