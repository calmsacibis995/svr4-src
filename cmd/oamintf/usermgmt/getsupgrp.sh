#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:usermgmt/getsupgrp.sh	1.2.1.1"

SUPGRP=`grep "[:,]${1}[,.]*" /etc/group |
	cut -d: -f1 |
	tr "\012" ","`

echo "${SUPGRP}" | sed -e 's/,$//'
