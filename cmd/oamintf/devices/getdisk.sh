#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:devices/getdisk.sh	1.2.4.1"
################################################################################
# 	Name: getdisk
#		
#	Desc: Get disk choices for partition or remove that doesn't have
#	      '/' or '/usr' or some other mounted file system.
#	
#	Arguments: $1 - device group
################################################################################
onefound=no
if [ "$1" ]
then
	
	list=`listdgrp $1`
	for disk in `getdev type=disk $list 2>/dev/null`
	do
		root=`getdev -a type=dpart mountpt=/`
		usr=`getdev -a type=dpart mountpt=/usr`

		IFS=" 	,"
		reserved=no
		for dpart in `devattr $disk dpartlist 2>/dev/null`
		do
			if [ $dpart = $usr ] || [ $dpart = $root ] 
			then
				reserved=yes
				break
			fi
			bdev=`devattr $dpart bdevice 2>/dev/null`
			if [ $? -eq 0 ]
			then
				/sbin/mount | grep ${bdev} > /dev/null 2>&1
				if [ $? -eq 0 ]
				then
					reserved=yes
					break
				fi
			fi
		done
		if [ "$reserved" = no ]
		then
			echo "$disk\072\c"; devattr $disk desc
			onefound=yes
		fi
			
	done
else
	for disk in `getdev type=disk 2>/dev/null`
	do
		root=`getdev -a type=dpart mountpt=/`
		usr=`getdev -a type=dpart mountpt=/usr`

		IFS=" 	,"
		reserved=no
		for dpart in `devattr $disk dpartlist 2>/dev/null`
		do
			if [ $dpart = $usr ] || [ $dpart = $root ] 
			then
				reserved=yes
				break
			fi
			bdev=`devattr $dpart bdevice 2>/dev/null`
			if [ $? -eq 0 ]
			then
				/sbin/mount | grep ${bdev} > /dev/null 2>&1
				if [ $? -eq 0 ]
				then
					reserved=yes
					break
				fi
			fi
		done
		if [ "$reserved" = no ]
		then
			echo "$disk\072\c"; devattr $disk desc
			onefound=yes
		fi
	done
fi

if [ "$onefound" = yes ]
then
	exit 0
else
	exit 1
fi
