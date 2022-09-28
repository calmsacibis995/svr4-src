#!
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)adm:filesave.sh	1.3.4.1"

#	The following prototype is meant as a guide in setting
#	up local filesave procedures.  It copies the BOOT, ROOT
#	and USR file systems from drive 0 to drive 1.

if [ $# != 1 ]
then
	echo usage: filesave packname
	exit 2
fi
don mhd 1
date >> /var/adm/log/filesave.log
dd if=/dev/boot0 of=/dev/boot1 bs=152b count=4
volcopy root /dev/rdsk/0s0 S3B000 /dev/rdsk/1s0 $1
volcopy usr /dev/rdsk/0s1 S3B000 /dev/rdsk/1s1 $1
echo FILESAVE COMPLETED >> /var/adm/log/filesave.log
doff mhd 1
