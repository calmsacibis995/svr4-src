#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)filemgmt:bin/indeftab.sh	1.1.3.1"
DEV=$1
FS=$2
if test -b $DEV
then
	BDEVICE="$DEV"
else
	BDEVICE=`devattr "$DEV"  bdevice 2>/dev/null`
	if test "$BDEVICE" = ""
	then
		BDEVICE="$DEV"
	fi
fi
while read bdev rdev mountp fstype fsckpass automnt mountopts
do
	case $bdev in
	'#'* | '' )
		continue;;
	'-')
		continue
	esac
	if test "$BDEVICE" = "$bdev" -a "$FS" = "$mountp"
	then
		echo "$mountopts" > /tmp/mntopts
		grep "-" /tmp/mntopts  > /dev/null
		if [ $? -eq 0 ]
		then 
			mntopts="read/write"
			setuid="no"
		fi
		grep "rw" /tmp/mntopts  > /dev/null
		if [ $? -eq 0 ]
		then 
			mntopts="read/write"
			setuid="no"
		fi
		grep "ro" /tmp/mntopts  > /dev/null
		if [ $? -eq 0 ]
		then 
			mntopts="read-only"
		fi
		setuid="yes"	
		grep  "nosuid" /tmp/mntopts > /dev/null
		if [ $? -eq 0 ]
		then 
			setuid="no"
		fi
		if [ "t" = "t$mntopts" ]
		then
			mntopts="read/write"
		fi
		echo $fstype $mntopts $setuid $automnt > /tmp/indeftab
		echo "true"
		exit 0
	else
		continue
	fi
done < /etc/vfstab
exit 1
