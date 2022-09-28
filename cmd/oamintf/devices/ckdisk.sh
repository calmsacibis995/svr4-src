#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:devices/ckdisk.sh	1.1.3.1"

################################################################################
#	Module Name: valdisk.sh
#	
#	Inputs:
#		$1 - group
#		$2 - device
#	
#	Description: Verify that a valid disk was entered.
################################################################################
for disk in  `getdev type=disk $2`
do
	root=`getdev -a type=dpart mountpt=/`
	usr=`getdev -a type=dpart mountpt=/usr`

	IFS=" 	,"
	reserved=no
	for dpart in `devattr $disk dpartlist`
	do
		if [ $dpart = $usr ] || [ $dpart = $root ] 
		then
			reserved=yes
			break
		fi
	done
	[ "$reserved" = no ] && exit 0
done

exit 1
