#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)disksetup:diskadd.sh	1.3.2.4"

# This script is used for adding disk drives to AT&T UNIX System V Release 4.0
# It runs adddisk or /etc/scsi/sadddisk to prompt the user for information 
# about the disk.  The diskadd program builds the following files:
# /tmp/partitions -- with the disk characteristics and partitions from the
#                    user,
# /tmp/addparts -- contains the names of additional partitions the user may
#                  want in addition to the required standard ones,
# /tmp/mkfs.data -- contains information about each filesystem which the user
#                   wants made.
#
# support is added to call the saddisk command if given a scsi device name 
# or if the device name is 1 and it is linked to the scsi dn.

trap 'trap "" 1 2 3 9 15;
	set +e;
	cd /;
	echo "You have canceled the diskadd program.  "
	rm -rf /var/spool/locks/.DISKADD.LOCK
exit 2' 1 2 3 15

if [ -f /var/spool/locks/.DISKADD.LOCK ]
then
	echo "The diskadd program is currently being run and cannot be run"
	echo "concurrently. Please retry this at a later time."
	exit 1
else
	>/var/spool/locks/.DISKADD.LOCK
fi
echo "You have invoked the diskadd utility. The purpose of this utility is"
echo "the set up of additional disk drives. This utility can destroy "
echo "the existing data on the disk. Do you wish to continue?"
echo "(Strike y or n followed by ENTER) "
read cont
if  [ "$cont" != "y" ] && [ "$cont" != "Y" ] 
then
	rm -rf /var/spool/locks/.DISKADD.LOCK
	exit 0
fi

set +e 
if [ -n "$1" ]
then
	drive=$1
else
	machid -s
	if [ "$?" = 1 ]
	then
		if [ -b /dev/dsk/c0t1d0s0 ]
		then
			drive=c0t1d0
		else
			# set to scsi to cause usage message
			drive=scsi 
		fi
	else
		# machine is esdi-based assume 2nd esdi drive
		drive=1
	fi
fi

case $drive in
1) dn=01; devnm=/dev/rdsk/1s0; t_mnt=/dev/dsk/1s;;
c?t?d?) # added for scsi devices

	slot=`echo $drive | cut -c2`
	tc=`echo $drive | cut -c4`
	drv=`echo $drive | cut -c6`
	# add check for valid args

	dn=${slot}${tc}${drv}
	devnm=/dev/rdsk/c${slot}t${tc}d${drv}s0
	t_mnt=/dev/dsk/c${slot}t${tc}d${drv}s;;

*) echo "usage: $0 [1] ( for Second Disk )"; 
   echo "usage: $0  cCtTdD  ( for SCSI Disks ) \n";
   echo "Issued with [1] or without an argument $0 will"
   echo "add the Second integral disk at /dev/rdsk/1s0."
   echo "For a SCSI disk the argument must be in the form:"
   echo "	Where C is the SCSI Host Adapter Controller number"
   echo "	      T is the Target ID on that Controller"
   echo "	and   D is the Device ID at that Target."
 
   rm -rf /var/spool/locks/.DISKADD.LOCK
   exit 1;; # added for scsi
esac
set +e
/etc/mount | grep ${t_mnt} > /dev/null 2>&1
if [ $? = 0 ]
then echo "The device you wish to add cannot be added.\nIt is already mounted as a filesystem."
     rm -rf /var/spool/locks/.DISKADD.LOCK
     exit 1
fi

###################################################################################
# If support for disks with > 1024 cyls (using a controller that doesn't provide 
# transparent logical translation) is required, the following eight lines should be
# uncommented. (This assumes the changes to disksetup and INSTALL were also made.)
# set +e
# disksetup -O $devnm
# dOerr=$?
# set -e
# if [ $dOerr = 11 ]		# disk params are changed
# then
# 	/sbin/disksetup -I $devnm
# fi
###################################################################################
set +x


/sbin/fdisk $devnm 
if [ $? != 0 ]
then
   echo "\n"
   echo "usage: $0 [1] ( for Second Disk )"; 
   echo "usage: $0  cCtTdD  ( for SCSI Disks ) \n";
   echo "Issued with [1] or without an argument $0 will"
   echo "add the Second integral disk at /dev/rdsk/1s0."
   echo "For a SCSI disk the argument must be in the form:"
   echo "	Where C is the SCSI Host Adapter Controller number"
   echo "	      T is the Target ID on that Controller"
   echo "	and   D is the Device ID at that Target."
   rm -rf /var/spool/locks/.DISKADD.LOCK
   exit 1
fi
 
	
echo "\nDisk partitioning complete."
if [ "$devnm" = "/dev/rdsk/1s0" ]
then
	sed '/\/dsk\/1s/d' /etc/vfstab >/etc/tmpvfstab
	mv /etc/tmpvfstab /etc/vfstab
fi
/sbin/disksetup -I $devnm
if [ $? != 0 ]
then
   echo "\n"
   echo "The Installation of the disk has failed."
   echo "Received error return value from /sbin/disksetup."
   echo "Please verify your disk is connected correctly."
   rm -rf /var/spool/locks/.DISKADD.LOCK
   exit 1
fi
/sbin/prtvtoc -e $devnm >/dev/null
# Run the mkdtab commands to update device.tab to include new disk
/usr/sadm/sysadm/bin/mkdtab
if [ -f /etc/scsi/scsimkdtab ]
then
	/etc/scsi/scsimkdtab
fi
rm -rf /var/spool/locks/.DISKADD.LOCK
echo "Diskadd for" disk$dn "DONE at" `date`
exit 0
