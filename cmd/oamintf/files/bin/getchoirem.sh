#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:files/bin/getchoirem.sh	1.1"

> /tmp/choices.$$
for des in `/usr/sadm/sysadm/bin/defdev | sort -u`;
do
	grep "$des" /tmp/choices.$$ > /dev/null 2>&1
	if [ $? != 0 ]
	then
		nam=`echo "$des" | sed 's/^.*dsk\///'`
		chr=`echo "$nam" | sed 's/^\(.\).*/\1/'`
		if [ "$chr" = "f" ]
		then
			devalias=`devattr $des alias`
			echo "$des\072$devalias"
 		elif [ "$chr" = "c" ]
		then
			slice=`echo "$nam" | sed 's/^.*s//'`
			device=`echo "$nam" | sed 's/s.*/s0/'`
			desc="`devattr /dev/rdsk/"$device" alias` slice $slice"
			echo "$des\072$desc"
		else
			echo "$des\072$des"
		fi
		echo "$des" >> /tmp/choices.$$
	fi
done
rm -f /tmp/choices.$$
