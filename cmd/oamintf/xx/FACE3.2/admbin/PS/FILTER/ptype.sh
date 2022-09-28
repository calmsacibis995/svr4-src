#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:xx/FACE3.2/admbin/PS/FILTER/ptype.sh	1.1.1.1"
echo "any" > /usr/tmp/ptype.$1
cut -f1 /usr/vmsys/admin/PS/PORTSET/database > /usr/tmp/ptype$$
for i in `cat /usr/tmp/ptype$$`
do
		echo $i >> /usr/tmp/ptype.$1
done
rm -f /usr/tmp/ptype$$
