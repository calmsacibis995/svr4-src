#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)filemgmt:bin/chkrun.sh	1.1.3.1"

echo "
   Please Wait, interactive file system check is in progress."
flags="-qq -k$$"	# flags for checkyn to implement [q] (quit)

trap 'exit 1' 1 2 15

pid=$$

arg1=${1}
fstype=${2}
bdrv=`devattr ${arg1} bdevice`
cdrv=`devattr ${arg1} cdevice`
pdrv=`devattr ${arg1} pathname`

if  [ $bdrv ] 
then ddrive=$bdrv
else if  [ $cdrv ] 
	then ddrive=$cdrv
	else if  [ $pdrv ] 
		then ddrive=$pdrv
		else 	
			echo "Error - ${arg1} does not have a device pathname" 
			exit 1
     	     fi
     fi
fi

ndrive="${arg1} drive"
l=`/usr/sbin/labelit -F ${fstype}  ${ddrive} 2>/dev/null`

eval `/usr/lbin/labelfsname "${l}"`

#Select:
#	interactive repair

#	Interrupting an fsck can be dangerous and cause strange messages,
#	therefore, they are ignored.
trap '' 2

/sbin/fsck -F ${fstype} ${ddrive}
exit=$?
trap 'exit 0' 2

if [ ${exit} -ne 0 ]
then
	echo "
   File system check did not finish successfully.
    " 
else
	echo "File system check completed successfully.\n"
fi
