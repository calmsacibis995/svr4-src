#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:edsysadm/edhelp.sh	1.3.2.1"
#
# if EDITOR is not set then default it to /usr/bin/ed.
#

if [ -z "`echo $EDITOR`" ]
then
	EDITOR=/usr/bin/ed
	echo "EDITOR has been set to '`echo $EDITOR`'."
	sleep 2
fi

hfname=`echo ${1} | sed -e 's/  *//'`

if [ -z "$hfname" ] 
then
	$EDITOR "Help"
else
	$EDITOR $1
fi
