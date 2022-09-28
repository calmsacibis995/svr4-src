#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:usermgmt/redeflt.sh	1.4.3.1"

#redeflt
# this will redefine the defaults

grp=`echo ${1} | cut -d, -f1`
basedir=${2}
skel=${3}
shnm=${4}
inact=$5
exp=$6

group=`echo ${grp} | sed -n -e "s/^\([^,]*\).*/\1/p"`

putusrdefs -g ${group} -b ${basedir} -k ${skel} -s ${shnm} -f "$inact" -e "$exp" >/tmp/dflterr 2>&1

if [ $? -eq 0 ]
then
	rm -f /tmp/dflterr
	exit 0
else
	exit 1
fi
