#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#!/sbin/sh
#ident	"@(#)initpkg:./mountall.sh	1.18.13.1"
USAGE="Usage:\nmountall [-F FSType] [-l|-r] [file_system_table]"
TYPES="all"
while getopts ?rlF: c
do
	case $c in
	r)	RFLAG="r";;
	l)	LFLAG="l";;
	F)	FSType=$OPTARG;
		if [ "$TYPES" = "one" ]
		then
			echo "mountall: more than one FSType specified"
			exit 2
		else
			TYPES="one"
		fi;
		case $FSType in
		?????????*) 
			echo "mountall: FSType $FSType exceeds 8 characters"
			exit 2
		esac
		;;
	\?)	echo "$USAGE" 1>&2; exit 2;;
	esac
done
oi=0
for i in 0 1 2 3 4 5 6 7 8 9
do
	if [ $i = $OPTIND ]
	then
		OPTIND=$oi
		break
	fi
	oj=0
	for j in 0 1 2 3 4 5 6 7 8 9
	do
		if [ $i$j = $OPTIND ]
		then
			if [ $j = 0 ]
			then
				OPTIND=${oi}9
			else
				OPTIND=$i$oj
			fi
			break 2
		fi
		oj=$j
	done
	oi=$i
done
shift $OPTIND
if [ "$RFLAG" = "r" -a "$LFLAG" = "l" ]
then
	echo "mountall: options -r and -l incompatible" 1>&2
	echo "$USAGE" 1>&2
	exit 2
fi
if [ $# -gt 1 ]
then
	echo "mountall: multiple arguments not supported" 1>&2
	echo "$USAGE" 1>&2
	exit 2
fi

# get file system table name and make sure file exists
case $1 in
	"-")	FSTAB=""
		;;
	"")	FSTAB=/etc/vfstab
		;;
	*)	FSTAB=$1
		;;
esac
if [ "$FSTAB" != ""  -a  ! -s "$FSTAB" ]
then
	echo "mountall: file system table ($FSTAB) not found"
	exit 1
fi

if [ \( "$FSType" = "rfs" -o "$FSType" = "nfs" \) -a "$LFLAG" = "l" ]
then
	echo "mountall: option -l and FSType are incompatible" 1>&2
	echo "$USAGE" 1>&2
	exit 2
fi
if [ \( "$FSType" = "s5" -o "$FSType" = "ufs" -o "$FSType" = "bfs" \) -a "$RFLAG" = "r" ]
then
	echo "mountall: option -r and FSType are incompatible" 1>&2
	echo "$USAGE" 1>&2
	exit 2
fi

#	file-system-table format:
#
#	column 1:	special- block special device or resource name
#	column 2: 	fsckdev- char special device for fsck 
#	column 3:	mountp- mount point
#	column 4:	fstype- File system type
#	column 5:	fsckpass- number if to be checked automatically
#	column 6:	automnt-	yes/no for automatic mount
#	column 7:	mntopts- -o specific mount options

#	White-space separates columns.
#	Lines beginning with \"#\" are comments.  Empty lines are ignored.
#	a '-' in any field is a no-op.

	exec < $FSTAB
	while  read special fsckdev mountp fstype fsckpass automnt mntopts 
	do
		case $special in
		'#'* | '')	#  Ignore comments, empty lines
				continue ;;
		'-')		#  Ignore no-action lines
				continue
		esac 

		if i386
		then
			if [ "$special" = "/dev/root" ]
			then continue
			fi
		fi
		
		if  [ "$FSType" ]
		then			# ignore different fstypes
			if [ "$FSType" != "$fstype" ]
			then
				continue
			fi
		fi

		if [ "$LFLAG" ]
		then
			if [ "$fstype" = "rfs"  -o  "$fstype" = "nfs" ]
			then
				continue
			fi
		fi
		if [ "$RFLAG" ]
		then
			if [ "$fstype" != "rfs" -a  "$fstype" != "nfs" ]
			then
				continue
			fi
		fi
		if [ "$automnt" != "yes" ]
		then
			continue
		fi
		if [ "$fstype" = "-" ]
		then
			echo "mountall: FSType of $special cannot be identified" 1>&2
			continue
		fi
		# 	Use mount options if any
		if  [ "$mntopts" != "-" ]
		then
			OPTIONS="-o $mntopts"
		else
			OPTIONS=""
		fi

		#	First check file system state and repair if necessary.

		if [ "$fsckdev" = "-" ]
		then
			/sbin/mount "-F" $fstype $OPTIONS $special $mountp
			continue
		fi

		msg=`/sbin/fsck "-m" "-F" $fstype $special 2>&1`
		case $? in
		0)	/sbin/mount "-F" $fstype $OPTIONS $special $mountp
			;;

		32)	echo "$msg\n\t $fsckdev is being checked" 1>&2
			if [ "$fstype" != "s5" ]
			then
				/sbin/fsck "-F" $fstype -y $fsckdev
			else 
				/sbin/fsck "-F" $fstype -y -t /var/tmp/tmp$$ -D $fsckdev
			fi
			/sbin/mount "-F" $fstype $OPTIONS $special $mountp
			;;

		33)	# already mounted
			#echo "$special already mounted"
			;;
		esac

	done
