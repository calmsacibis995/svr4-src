#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)filemgmt:bin/numblocks.sh	1.1.3.1"

DEVICE=$1
#get the block device
if [ ! -b $DEVICE ]
then
	BDEVICE=`devattr "$DEVICE" bdevice  2>/dev/null`
else
	BDEVICE="$DEVICE"
fi
# if device is diskette use defaults else read size from prtvtoc o/p.
DEVALIAS=`devattr $DEVICE alias | sed 's/^\(.*\).$/\1/'`
if [ "$DEVALIAS" = "diskette" ]
then
	#blocksize=1422
	blocksize=2048
else
	blocksize=`devattr $BDEVICE capacity`

fi
echo "$blocksize"
exit 0
