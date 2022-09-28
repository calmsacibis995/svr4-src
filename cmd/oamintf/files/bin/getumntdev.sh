#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:files/bin/getumntdev.sh	1.1"

/sbin/mount | /usr/bin/cut -d" " -f1,2,3 | \
while read MNTNAME ON DEVNAME
do
	nam=`echo "$DEVNAME" | sed 's/^.*dsk\///'`
	chr=`echo "$nam" | sed 's/^\(.\).*/\1/'`
	if [ "$chr" = "c" ]
	then
		slice=`echo "$nam" | sed 's/^.*s//'`
		device=`echo "$nam" | sed 's/s.*/s0/'`
		DESC="`devattr /dev/rdsk/"$device" alias` slice $slice"
	else
		DESC="$DEVNAME"
	fi
	echo "$MNTNAME\072$ON $DESC"
done

