#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#!/sbin/sh
#ident	"@(#)initpkg:./umountall.sh	1.4.14.1"
USAGE="Usage:\numountall [-F FSType] [-k] [-l|-r] "
FSTAB=/etc/vfstab
FSType=
kill=
FCNT=0
CNT=0
while getopts ?rlkF: c
do
	case $c in
	r)	RFLAG="r"; CNT=`/usr/bin/expr $CNT + 1`;;
	l)	LFLAG="l"; CNT=`/usr/bin/expr $CNT + 1`;;
	k) 	kill="yes";;
	F)	FSType=$OPTARG; 
		case $FSType in
		?????????*) 
			echo "umountall: FSType $FSType exceeds 8 characters"
			exit 2
		esac;
		FCNT=`/usr/bin/expr $FCNT + 1`;;
	\?)	echo "$USAGE" 1>&2; exit 2;;
	esac
done
shift `/usr/bin/expr $OPTIND - 1`
if test $FCNT -gt 1
then
	echo "umountall: more than one FStype specified" 1>&2
	exit 2
fi
if test $CNT -gt 1
then
	echo "umountall: options -r and -l incompatible" 1>&2
	echo "$USAGE" 1>&2
	exit 2
fi
if test $# -gt 0
then
	echo "umountall: arguments not supported" 1>&2
	echo "$USAGE" 1>&2
	exit 2
fi
if test \( "$FSType" = "rfs" -o "$FSType" = "nfs" \) -a "$LFLAG" = "l"
then
	echo "umountall: option -l and FSType are incompatible" 1>&2
	echo "$USAGE" 1>&2
	exit 2
fi
if test \( "$FSType" = "s5" -o "$FSType" = "ufs" -o "$FSType" = "bfs" \) -a "$RFLAG" = "r"
then
	echo "umountall: option -r and FSType are incompatible" 1>&2
	echo "$USAGE" 1>&2
	exit 2
fi

/sbin/mount -v  |
	/usr/bin/sort -r  |
	while read dev dum1 mountp dum2 fstype mode dum3
	do
		case "${mountp}" in
		/  | /stand | /proc | /dev/fdfs | '' )
			continue
			;;
		* )
			if [ "$FSType" ]
			then
				if test "$FSType" != "$fstype"
				then
					continue
				fi
			fi
			if test "$LFLAG" = "l"
			then
				if test "$fstype" = "rfs" -o "$fstype" = "nfs"
				then
					continue
				fi
			fi
			if test "$RFLAG" = "r"
			then
				if test "$fstype" != "rfs" -a "$fstype" != "nfs"
				then
					continue
				fi
			fi
			if [ ${kill} ]
			then
				/usr/sbin/fuser -k ${dev}
				/usr/bin/sleep 10
			fi
			/sbin/umount ${dev}
		esac
	done 
