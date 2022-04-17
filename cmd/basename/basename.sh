#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)basename:basename.sh	1.9"
if [ $# -gt 2 ]
then
	echo >&2 "Usage:  basename [ path [ suffix-pattern ] ]"
	exit 1
fi
#	If no first argument or first argument is null, make first argument
#	"."  Add beginning slash, then remove trailing slashes, then remove 
#	everything up through last slash, then remove suffix pattern if 
#	second argument is present.
#	If nothing is left, first argument must be of form //*, in which
# 	case the basename is /.
exec /usr/bin/expr \
	"/${1:-.}" : '\(.*[^/]\)/*$' : '.*/\(..*\)' : "\\(.*\\)$2\$"  \|  \
	"/${1:-.}" : '\(.*[^/]\)/*$' : '.*/\(..*\)'    \|  \
	"/${1:-.}" : '.*/\(..*\)' 
