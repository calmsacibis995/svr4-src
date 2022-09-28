#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:files/bin/isoam.read.sh	1.1.1.1"

# Post a message if an attempt to read in a preSVR4 package is made

echo "
	You may not spool a preSVR4 package. The read_in function is
	applicable only to 4.0 style packages.
"

exit 0
