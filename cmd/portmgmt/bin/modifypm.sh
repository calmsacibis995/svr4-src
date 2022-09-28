#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)portmgmt:bin/modifypm.sh	1.2.2.1"

# modifypm - modify port monitor information in sactab

SACTAB=/etc/saf/_sactab
TMPFILE=/var/tmp/$$sactab

sed "s/^$1:/###$1:/" $SACTAB > $TMPFILE
cp $TMPFILE $SACTAB
sacadm -x

flags=""
if [ "$3" = No ]
then
	flags="x"
fi
if [ $4 = DISABLED ]
then
	flags="d$flags"
fi

tmpline="$1:$2:$flags:$5:$6	#$7"

line=`echo "$tmpline"| sed 's/\\//\\\\\\//g'`

sed "s/###$1:.*$/$line/" $TMPFILE > $SACTAB  
sacadm -x
rm $TMPFILE
