#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)filemgmt:bin/getfilesize.sh	1.1.5.1"

# getfilesize.sh

echo "" >/tmp/file.size

fsys=${1}
nfiles=${2}

cd ${fsys}

a="`expr 0${nfiles} "*" 10`"
du -a  |
	sort -bnr +0 -1  |
	sed -n 1,0${a}'s:^[0-9]*	\./:ls -ldsu :p'  |
	sh -  |
	grep -v '^ *[0-9][0-9]* d'  |
	sed -n 1,0${nfiles}p  |
	sort -bnr +5 -6 |
	cut -c21-29,37- > /tmp/$$filesize

afiles="`cat /tmp/$$filesize  |  wc -l  |  cut -c5-`"

if [ "${nfiles}" -ne "${afiles}" ]
then
	nfiles=${afiles}
	echo "
   There are ${nfiles} files in `pwd`.\n" >/tmp/file.size
fi
echo "\nThe ${nfiles} largest files in `pwd`:\n 
       file size   date of
owner (characters) last access  filename
----- ------------ ------------ --------" >>/tmp/file.size
cat /tmp/$$filesize >>/tmp/file.size
rm /tmp/$$filesize 
