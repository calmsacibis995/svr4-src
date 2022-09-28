#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)spell:compress.sh	1.4"
#	compress - compress the spell program log

trap 'rm -f /usr/tmp/spellhist;exit' 1 2 3 15
echo "COMPRESSED `date`" > /usr/tmp/spellhist
grep -v ' ' /var/adm/spellhist | sort -fud >> /usr/tmp/spellhist
cp /usr/tmp/spellhist /var/adm
rm -f /usr/tmp/spellhist
