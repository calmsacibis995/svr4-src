#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)dirname:dirname.sh	1.7"
if [ $# -gt 1 ]
then
	echo >&2 "Usage: dirname [ path ]"
	exit 1
fi
#	First check for pathnames of form //*non-slash*/* in which case the 
#	dirname is /.
#	Otherwise, remove the last component in the pathname and slashes 
#	that come before it.
#	If nothing is left, dirname is "."
exec /usr/bin/expr \
	"${1:-.}/" : '\(/\)/*[^/]*//*$'  \| \
	"${1:-.}/" : '\(.*[^/]\)//*[^/][^/]*//*$' \| \
	.
