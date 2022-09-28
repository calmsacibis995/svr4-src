#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)filemgmt:bin/filesearch.sh	1.1.5.1"

# Remember to put search.text into a /tmp file 


fsys="${1}"
days="${2}"

if test  -d ${fsys}
	then
		cd ${fsys}
		flist=`find . -mtime +${days} -print`
		if test -z "${flist}"
		then
			echo "
   There are no files older than ${days} days in" `pwd` >/tmp/search.text
			exit 1
		else

echo "\nFILES NOT MODIFIED IN THE LAST ${days} DAYS IN `pwd`:\n
       	 file size   date of
owner   (characters) last access  filename
-----   ------------ ------------ --------" >/tmp/search.text
				echo ${flist} | sort  | xargs ls -dl  | cut -c16-24,30-54,57- >>/tmp/search.text
				echo >>/tmp/search.text
			exit 0
		fi

	else
		exit 1

fi
