#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:xx/FACE3.2/admbin/PS/FILTER/pname.sh	1.1.1.1"
echo "any" > /usr/tmp/pname.$1
ls /usr/spool/lp/admins/lp/printers > /usr/tmp/name$$
if [ -s /usr/tmp/name$$ ]
then
for i in `cat /usr/tmp/name$$`
do
		echo $i >> /usr/tmp/pname.$1
done
fi
rm -f /usr/tmp/name$$
