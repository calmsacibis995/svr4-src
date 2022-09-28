#!
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)adm:tapesave.sh	1.3.4.1"

#	This is a prototype file for tapesaves of file systems.
#	It may need to be modified for local use.
#	It will copy the named file system onto tape drive 0.

echo 'Enter file system name (e.g., usr):'
read f
echo 'Enter device name (e.g., /dev/rdsk/1s0):'
read d
echo 'Enter pack volume label (e.g., p0045):'
read v
t=/dev/rmt/0m
echo 'Enter tape volume label (e.g., t0005):'
read l

/usr/sbin/labelit  $t
if test  $? -gt 0
then
  /usr/sbin/labelit $t $f  $l -n
  if test $? -gt 0
  then
    exit 1
  fi
fi
/usr/sbin/volcopy $f $d $v $t $l
