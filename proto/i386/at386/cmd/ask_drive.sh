#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

:

#ident	"@(#)proto:i386/at386/cmd/ask_drive.sh	1.3"

# if no 2nd drive, there's no sense in asking which drive.
flop_num
if [ $? = 2 ] 
then 
	while true 
	do 
		echo "Enter Floppy Drive Number ((default)0,1):  \c" 
		read ans 
		if [ "$ans" = "1" ] 
		then 
			DRIVE=1 
			break 
		elif [ "$ans" = "" -o "$ans" = "0" ] 
		then 
			DRIVE=0
			break
		fi 
	done 
else
	DRIVE=0
fi

# get drive device name
while true 
do 
	cat <<!!!
Enter Density of Floppy:
	1) 1.2 MB  (f${DRIVE}q15d)
	2) 1.44 MB (f${DRIVE}3h)
	3) 360 Kb  (f${DRIVE}d9d)
	4) 720 Kb  (f${DRIVE}3d)
 	5) none of the above

!!!
	echo "Please enter #(1-5), default 1: \c"
	read ans
	case "$ans" in
		1 | "" )
			FDRIVE=f${DRIVE}q15d 
			BLKCYLS=30 
			BLKS=2370
			;;
		2 )
			FDRIVE=f${DRIVE}3h
			BLKCYLS=36 
			BLKS=2844
			;;
		3 )
			FDRIVE=f${DRIVE}d9d 
			BLKCYLS=18 
			BLKS=702
			;;
		4 )
			FDRIVE=f${DRIVE}3d 
			BLKCYLS=18 
			BLKS=1422
			;;
		5 )
			echo "Enter device name (e.g. f0q15d): \c"
			read FDRIVE
			echo "How many blocks/cylinder? \c"
			read BLKCYLS
			echo "How many blocks on device (minus 1st cyl)? \c"
			read BLKS
			;;
		* )
			echo "Invalid."
			continue
	esac
	break
done
echo "${FDRIVE}\t${BLKCYLS}\t${BLKS}" >&2

