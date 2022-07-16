#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)mbus:cmd/disksetup/diskadd.sh	1.3.2.2"

# This script is used for adding disk drives to AT&T UNIX System V Release 4.0
# It runs adddisk to prompt the user for information 
# about the disk.  
#

# Functions for formatting drive to be added
Do_print_dsk() {
	echo
	echo "Heads:             $heads"
	echo "Cylinders:         $cyls"
	echo "Sectors per track: $secs"
	echo "Bytes per Sector:  $gran"
	echo "Interleave:        $intrl"
	echo "\nAre these correct? (y/[n]) \c"
}
Do_num_arg() {
	args=$1

	case $args in
		*[a-z]*)	return 1;;
		*[A-Z]*)	return 1;;
			 '')	return 1;;
	esac
	return 0
}
Do_dsk_args(){
heads=
cyls=
gran=1024
secs=
intrl=
	echo
	while :
	do
		echo 
		while [ -z "$heads" ]
		do
			echo "Enter Number of heads: \c"
			read theads
			Do_num_arg $theads && heads=$theads
		done
		while [ -z "$cyls" ]
		do
			echo "Enter Number of cylinders: \c"
			read tcyls
			Do_num_arg $tcyls && cyls=$tcyls
		done
		while [ -z "$secs" ]
		do
			echo "Enter Number of sectors per track: \c"
			read tsecs
			Do_num_arg $tsecs && secs=$tsecs
		done
		while [ -z "$gran" ]
		do
			echo "Enter Number of bytes per sector: \c"
			read tgran
			Do_num_arg $tgran && gran=$tgran
		done
		while [ -z "$intrl" ]
		do
			echo "Enter Disk Interleave: \c"
			read tintrl
			Do_num_arg $tintrl && intrl=$tintrl
		done
		Do_print_dsk
		read ans
		case $ans in
			y*)	echo ;PARAMS_SET=1;break;;
		esac
		heads=
		cyls=
		secs=
		gran=
		intrl=
		echo
	done

}

echo "You have invoked the diskadd utility.  The purpose of this utility is the"
echo "set up of additional disk drives.  This utility can destroy the existing"
echo "data on the disk.  Do you wish to continue?  (Strike y or n followed by ENTER)"
read cont
if  [ "$cont" != "y" ] && [ "$cont" != "Y" ] 
then
	exit 0
fi

drive=${1:-1}
format=0

echo
echo "Would you like to format drive $drive before adding it?  (To"
echo "format, you will be required to know the geometry of your drive.)"
echo "(Strike y or n followed by ENTER)"
read cont
echo
if  [ "$cont" = "y" ] || [ "$cont" = "Y" ] 
then
	format=1
fi

dn="0$drive"
devnm=/dev/rdsk/${drive}s0
t_mnt=/dev/dsk/${drive}s

case $drive in
	1|2|3 ) : ;;
	*) echo "usage: $0 [ -f ] [ drive_number ]"
	   exit 1;; 
esac

/etc/mount | grep ${t_mnt} > /dev/null 2>&1
if [ $? = 0 ]
then echo "The device you wish to add cannot be added.\nIt is already mounted as a filesystem."
     exit 1
fi
##################
minor_offset=`expr $drive \* 16`
for i in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
do
	rm -f $ROOT/dev/dsk/${drive}s${i} 
	rm -f $ROOT/dev/rdsk/${drive}s${i}
	minor=`expr $minor_offset + $i `
	mknod $ROOT/dev/dsk/${drive}s${i} b 0 $minor
	mknod $ROOT/dev/rdsk/${drive}s${i} c 0 $minor
done
##################

if [ "$devnm" = "/dev/rdsk/1s0" ]
then
	sed '/\/dsk\/1s/d' /etc/vfstab >/etc/tmpvfstab
	mv /etc/tmpvfstab /etc/vfstab
fi

if [ $format -eq 1 ] 
then
	Do_dsk_args
	hdformat -i $intrl -c $cyls -f $heads -d $gran -s $secs \
		/dev/rdsk/${drive}s0  
fi
/sbin/disksetup -I $devnm
if [ $? != 0 ]
then
   echo "\n"
   echo "The Installation of the Disk has failed."
   echo "Received error return value from /sbin/disksetup."
   echo "Please verify your disk is connected correctly."
   rm -rf /var/spool/locks/.DISKADD.LOCK
   exit 1
fi
/sbin/prtvtoc -e $devnm >/dev/null
echo "Diskadd for" disk$dn "DONE at" `date`
exit 0
