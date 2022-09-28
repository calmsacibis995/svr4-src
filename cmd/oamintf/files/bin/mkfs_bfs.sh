#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:files/bin/mkfs_bfs.sh	1.2.1.1"
DEVICE=$1
BLOCKS=$2
INODES=$3
LABEL=$4
MOUNTP=$5
echo "" > /tmp/make.out
if [ ! -b $DEVICE ]
then
	BDEVICE=`devattr "$DEVICE" bdevice 2>/dev/null`
else
	BDEVICE=$DEVICE
fi
if /sbin/mkfs -F bfs $BDEVICE $BLOCKS:$INODES 2>/tmp/mkerr$$
then
	echo "The file system was created successfully." >> /tmp/make.out
else
	echo "The file system could not be created:\n" >> /tmp/make.out
	cat /tmp/mkerr$$ >> /tmp/make.out
	/bin/rm /tmp/mkerr$$ 2>/dev/null
	exit 1
fi

if [ "$LABEL" != "NULL" ]
then
	/sbin/labelit -F bfs "$BDEVICE" "$LABEL" 2>/dev/null
	echo "The new file system has been labelled $LABEL." >> /tmp/make.out
fi

if [ "$MOUNTP" != "" ]
then
	if mount -F bfs $BDEVICE $MOUNTP 2> /tmp/mnterr$$
	then
		echo "File system successfully mounted as $MOUNTP." >> /tmp/make.out
	else
		echo "File system could not be mounted as \"$MOUNTP\":" >> /tmp/make.out
		cat /tmp/mnterr$$ >> /tmp/make.out
		/bin/rm /tmp/mnterr$$ 2>/dev/null
	fi

fi
exit 0
