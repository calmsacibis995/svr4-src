#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)oamintf:files/bin/isoam.ins.sh	1.1"

# Determine if a package is an installable OAM style package; RC=0 means yes

#
# Test for spooled type
#
if [ -d "${1}" ]
then
	echo true
	exit 0
fi

DEVICE=`devattr ${1} bdevice 2>/dev/null`
if [ -z "${DEVICE}" ]
then
	echo "invalid"
	exit 1
fi

#
# Test for datastreaming type 4.0 package
#
seq=`dd if=${DEVICE} bs=512 count=1 2>/dev/null`
if [ "20" = `expr "${seq}" : "# PaCkAgE DaTaStReAm"` ]
then
	echo true
	exit 0
fi

#
# Test for mountable type 4.0 package
#
/sbin/mount ${DEVICE} /install -r > /dev/null 2>&1
if [ -f `echo /install/*/pkginfo | cut -f1 -d' '` ]
then
	/sbin/umount /install > /dev/null 2>&1
	echo true
	exit 0
fi
/sbin/umount /install > /dev/null 2>&1

echo false
exit 0
