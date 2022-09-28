#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)filemgmt:bin/ckerrs.sh	1.1.3.1"

echo "" > /tmp/check.err

flags="-qq -k$$"	# flags for checkyn to implement [q] (quit)

trap 'exit 1' 1 2 15
fstype=$2
#fsname=$3
pid=$$

bdrv=`devattr ${1} bdevice`
cdrv=`devattr ${1} cdevice`
pdrv=`devattr ${1} pathname`
if  [ $bdrv ] 
then ddrive=$bdrv
else 
	if  [ $cdrv ] 
	then ddrive=$cdrv
	else if  [ $pdrv ] 
 	     then ddrive=$pdrv
	     else 	
		echo "   Error - ${1} does not have a device pathname" >>/tmp/check.err
		exit 1
     	     fi
     	fi
fi

ndrive="${1} drive"
bddrive=${bdrv}

unset getit question writechk nochecks mounted

trap 'exit 1' 1 2
trap "	trap '' 1 2
	kill ${pid};  exit 1" 15
flags="-q q -k $$"

#Determine, with some effort, whether the device is already mounted.
#Since the same major/minor number of a block device may have several
#names, we must use them to determine if the drive is mounted.
#See what the major/minor number of the requested block device is, then
#look for that among the mounted devices.

majmin=`ls -l ${bddrive} 2>/dev/null  |
	sed -n 's/.*\( [0-9]\{1,\}, *[0-9]\{1,\} \).*/\1/p'`

#It is possible there is no corresponding block device for a character dev

if [ -n "${majmin}" ]
then
	mounted=`/sbin/mount  |  cut -d' ' -f3  |  xargs ls -l 2>/dev/null  |
		sed -n "/${majmin}/s/.* \\(.*\\)/\\1/p"`
fi
if [ -n "${mounted}" ]
then
	filesys=`/sbin/mount  |
		sed -n "\\; on ${mounted} ;s;^\\([^ ]*\\) .*;\\1;p"`
	echo >>/tmp/check.err "   The medium in the ${ndrive} is already mounted as the
   ${filesys} file system.  This command cannot run with it mounted.
   Before you can use the ${ndrive} for this purpose, you
   must unmount the medium now in use.
   Unmount it and try again."

	exit 1
fi

#	dd checks that the file is openable for reading.
if  dd count=1 if=${ddrive} of=/dev/null 2>/dev/null
then
	:
else
	echo >>/tmp/check.err '
   The medium may not be properly inserted,
   the drive door may not be closed, the medium may not be formatted,
   the medium may be in upside-down, or there is some other problem.
   Check it and try again.
'
	exit 1
fi

#/sbin/fsck -F ${fstype}  ${ddrive} >/dev/null 2>&1
#if [ $? != 0 ]
#then
#	echo >>/tmp/check.err '
#  	Error: the File System Status Command Failed.'
#	exit 1
#fi

#trap 'exit 1' 1 2 15
#ERR=0
#mounted=`/sbin/mount | sed 's; .*;;'`
#if echo "$mounted" | grep "${fsname}\$"
#then
#	ERR=3
#fi

#if [ $ERR -ne 0 ]
#then
#	echo "   Unable to mount file system '${fsname}' to complete checking." >>/tmp/check.err
#	case "$ERR" in
#	"3") echo "   A file system with that name is already mounted." >>/tmp/check.err
#		;;
#	*) echo "   Unknown error $ERR." >>/tmp/check.err
#		;;
#	esac
#  exit 1
#fi
