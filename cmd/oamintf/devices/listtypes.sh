#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:devices/listtypes.sh	1.1.4.1"
# print list of device types supporting attribute $1 along with the
# corresponding descriptions

devlist="`/usr/bin/getdev -a display=true $1:*`"

if [ -z "$devlist" ]
then
	exit 1
fi

typelist=`for device in $devlist
do
	/usr/bin/devattr $device type
done | sort -u`

tmpfile=/tmp/$$

for type in $typelist
do
	echo "${type}:\c" >> $tmpfile
	devlist=`/usr/bin/getdev type=$type`
	for device in $devlist
	do
		/usr/bin/devattr $device desc >> $tmpfile
		break
	done
done
/usr/bin/cat $tmpfile
rm -f $tmpfile
