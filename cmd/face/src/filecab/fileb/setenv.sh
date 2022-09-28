#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)face:src/filecab/fileb/setenv.sh	1.1"
#

# usage: setenv [-f file] [-r variable] [variable=value] ...
# sets variables in files

file=$HOME/pref/.environ
var=
while [ "$#" -gt 0 ]
do
	case "$1" in
	-f)
		file="$2"
		shift
		;;
	-f*)
		file="`expr \"$1\" : '-f\(.*\)'`"
		;;
	-r)
		var="$2"
		echo "`grep \"^$var=\" \"$file\" 2>/dev/null`" | sed -e "s/^$var=//"
		shift
		;;
	-r*)
		var="`expr \"$1\" : '-r\(.*\)'`"
		echo "`grep \"^$var=\" \"$file\" 2>/dev/null`" | sed -e "s/^$var=//"
		;;
	*)
		var="`expr \"$1\" : '\([^=]*\)=.*'`"
		(expr "$1" : '\(.*\)' | sed -e 's/\\[ncrbft0]/\\&/g' -e 's/$/\\n/' | tr -d '\012'; echo) | sed -e 's/\\n$//' > /tmp/setenv$$
		grep -v "^$var=" "$file" >> /tmp/setenv$$ 2>/dev/null
		cp /tmp/setenv$$ "$file"
		rm -f /tmp/setenv$$
		;;
	esac
	shift
done
