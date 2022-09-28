#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)filemgmt:bin/unmountfs.sh	1.1.3.1"
tmpfile=${2}

bdrv=`devattr ${1} bdevice`
cdrv=`devattr ${1} cdevice`
pdrv=`devattr ${1} pathname`
if  [ $bdrv ] 
then ddrive=$bdrv
else if  [ $cdrv ] 
	then ddrive=$cdrv
	else if  [ $pdrv ] 
		then ddrive=$pdrv
		else 	
			echo "   Error - ${1} does not have a device pathname" >>/tmp/$tmpfile
			exit 1
     	     fi
     fi
fi

ndrive="${1} drive"

bddrive=${bdrv}

mounted=`/sbin/mount | sed -n "\\;"$ddrive";s;^\([^ ]*\) on "$ddrive" .*;\1;p"`

msg=`/sbin/umount ${ddrive:?} 2>&1`

case "${msg}" in
'' )
	echo	${mounted} ${mounted:+'unmounted. '} \
		"   File System has been unmounted successfully.\n" >/tmp/$tmpfile
	;;
*' busy' )
	echo " \
   The file system is \"busy\" which means that either some command\
   is using files under ${mounted:-it} or someone is logged in and currently\
   in a directory within the file system." >/tmp/$tmpfile
	exit 1
	;;
*' not mounted' )
	echo "   /sbin/umount got '${msg}'" >/tmp/$tmpfile
	exit 1
	;;
* )
	echo "   /sbin/umount got '${msg}'" >/tmp/$tmpfile
	exit 1
esac
exit 0

