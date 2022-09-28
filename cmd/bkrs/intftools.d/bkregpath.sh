#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)bkrs:intftools.d/bkregpath.sh	1.1"

if [ ! -f /tmp/_bkreg_$1 ]
then
	brfindtab bkreg;
else
	cat /tmp/_bkreg_$1;
fi;
