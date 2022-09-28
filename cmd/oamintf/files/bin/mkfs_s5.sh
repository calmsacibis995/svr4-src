#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)filemgmt:bin/mkfs_s5.sh	1.1.3.1"
PROTO=$1
BLOCKSIZE=$2
DEVICE=$3
BLOCKS=$4
INODES=$5
LABEL=$6
MOUNTP=$7
echo "" > /tmp/make.out
if [ ! -b $DEVICE ]
then
	BDEVICE=`devattr "$DEVICE" bdevice 2>/dev/null`
else
	BDEVICE=$DEVICE
fi
if [ "$PROTO" != "NULL"  ]
then
	if  /sbin/mkfs -F s5 -b $BLOCKSIZE $BDEVICE $PROTO 2>/tmp/mkerr$$
	then
		echo "The file system was created successfully." >> /tmp/make.out
	else
		echo "The file system could not be created:\n" >> /tmp/make.out
		cat /tmp/mkerr$$ >> /tmp/make.out
		/bin/rm /tmp/mkerr$$ 2>/dev/null
		exit 1
		
	fi
else
	if /sbin/mkfs -F s5 -b $BLOCKSIZE $BDEVICE $BLOCKS:$INODES 2>/tmp/mkerr$$
	then
		echo "The file system was created successfully." >> /tmp/make.out
	else
		echo "The file system could not be created:\n" >> /tmp/make.out
		cat /tmp/mkerr$$ >> /tmp/make.out
		/bin/rm /tmp/mkerr$$ 2>/dev/null
		exit 1
	fi
fi
if [ "$LABEL" != "NULL" ]
then
	/sbin/labelit -F s5 "$BDEVICE" "$LABEL" 2>/dev/null
	echo "The new file system has been labelled $LABEL." >> /tmp/make.out
fi

if [ "$MOUNTP" != "" ]
then
	if mount -F s5 $BDEVICE $MOUNTP 2> /tmp/mnterr$$
	then
		echo "File system successfully mounted as $MOUNTP." >> /tmp/make.out
	else
		echo "File system could not be mounted as \"$MOUNTP\":" >> /tmp/make.out
		cat /tmp/mnterr$$ >> /tmp/make.out
		/bin/rm /tmp/mnterr$$ 2>/dev/null
	fi

fi
exit 0
