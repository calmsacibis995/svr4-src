#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)filemgmt:bin/checkfilsys.sh	1.1.3.1"
FSTYPE=$1
BDEV=`devattr $2 bdevice 2>/dev/null`
FSYS=$3
RW=$4
SUID=$5

/bin/rm /tmp/checkfilsys 2>/dev/null

if [ ! "$BDEV" ]
then
	BDEV=$2
fi
if [ "$RW" = "read-only" ]
then
	RW="-r"
else
	RW=""
fi
if [ "$SUID" = "yes" ]
then
	SUID="-o suid"
else
	if [ "$SUID" = "no" ]
	then
		SUID="-o nosuid"
	else
		SUID=""
	fi
fi
/sbin/fsck -F $FSTYPE -m $BDEV 2>&1 >/dev/null
case $? in
0)
	/sbin/mount -F $FSTYPE $RW $SUID $BDEV $FSYS >/dev/null 2> /tmp/mnterr$$
	if [ $? -eq 0 ]
	then
		echo "The file system was mounted successfully." >> /tmp/checkfilsys
	else
		echo "The file system could not be mounted:\n" >> /tmp/checkfilsys
		cat /tmp/mnterr$$ >>/tmp/checkfilsys
		/bin/rm /tmp/mnterr$$ 2>/dev/null
	fi;;
32)  
	echo "The file system needs checking. Use the\nCheck Task to check the file system." >> /tmp/checkfilsys;;
33) 
  	echo "The file system is already mounted." >> /tmp/checkfilsys;;
*)
	echo "The file system could not be checked.\nPlease check the device and try again." >> /tmp/checkfilsys
esac
exit 0
