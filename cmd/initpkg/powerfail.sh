#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)initpkg:./powerfail.sh	1.1.5.1"
if $1/bin/u3b2 || $1/bin/i386
then echo "#	powerfail
" >powerfail
fi
