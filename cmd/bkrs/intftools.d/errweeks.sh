#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)bkrs:intftools.d/errweeks.sh	1.3.3.1"
PERIOD=`getrpd $2`
if [ $PERIOD -eq 1 ]
then
	echo "Enter '1' (the only valid week) or 'demand' or '$1'."
else
	echo "Enter a list of week ranges (weeks 1-$PERIOD) or 'demand' or '$1'."  
fi
exit 0
