#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)lp.admin:width.sh	1.1"

CSET=`infocmp "$1" | grep "cols"`

if [ "$?" != "0" ]
then
	exit 1
fi

echo $CSET | cut -f4 -d"," | sed -e 's/ cols#//' > ${datafile}width
exit 0

