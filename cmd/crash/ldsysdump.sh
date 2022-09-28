#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)crash:ldsysdump.sh	1.1.3.3"
# load a system memory image dump from tape or floppy to a file

# shell variables used:
#
#       DEV     "f" = floppy; "t" = tape
#
#       IF      input device file used by dd
#       BS      block size used by dd, in 512 byte units
#       COUNT   number of blocks per disk/tape
#
#       NB      number of BS size blocks on tape/disk
#       N       number of BS size blocks of memory to copy


if [ $# -ne 1 ]
then
	echo 'usage: /etc/ldsysdump file'
	exit
fi

while :
do
	echo '\nIs the dump on:'
	echo '  1 - low density 5.25" (360K) diskettes'
	echo '  2 - high density 5.25" (1.2M) diskettes'
	echo '  3 - low density 3.5" (720K) diskettes'
	echo '  4 - high density 3.5" (1.44M) diskettes'
	if [ -r /dev/rmt/c0s0 ]
	then
		echo '  t - Cartridge tape'
	fi
	if [ -r /dev/scsi/qtape1 ]
	then
		echo '  s - SCSI cartridge tape'
	fi
	echo '  n - no, QUIT'
	echo '> \c'
	read ans
	case $ans in
	  1 )   BS=18 NB=40     DEV=f ; break ;;
	  2 )   BS=30 NB=80     DEV=f ; break ;;
	  3 )   BS=18 NB=80     DEV=f ; break ;;
	  4 )   BS=36 NB=80     DEV=f ; break ;;
	  t )   IF=/dev/rmt/c0s0     BS=32 NB=10000  DEV=t ;
		if [ -r $IF ]; then break; fi ;;
	  s )   IF=/dev/scsi/qtape1     BS=32 NB=10000  DEV=t ;
		if [ -r $IF ]; then break; fi ;;
	  n )   exit ;;
	esac
	echo '???'
done

if [ "$DEV" = "f" ]
then
/sbin/flop_num
if [ $? -gt 1 ]
then
	while true
	do
		echo "\nThis system has two floppy drives.\n\
Strike ENTER to load dump from drive 0\n\
or 1 to load dump from drive 1.  \c"
		read ans
		if [ "$ans" = 1 ]
		then
			IF=/dev/rdsk/f1t
			break
		elif [ "$ans" = "" -o "$ans" = 0 ]
		then
			IF=/dev/rdsk/f0t
			break
		fi
	done
else
	IF=/dev/rdsk/f0t
fi
fi

# while :
# do
#	echo 'How many megabytes of memory image do you want to load?'
#	echo 'Enter decimal integer or "q" to quit. > \c'
#	read ans
#	case $ans in
#	  q )   exit ;;
#	esac
#	N=`expr \( $ans \* 2048 + $BS - 1 \) / $BS`
#	case $? in
#	  0 )   break;;
#	esac
#	echo '???'
# done

SKIP=0
COUNT=$NB
N=-1
OLIM=`ulimit`

{   while [ $N != 0 ]
    do
	echo 'Insert \c' >&2
	case $DEV in
	  f )   echo 'diskette \c' >&2 ;;
	  t )   echo 'tape cartridge \c' >&2 ;;
	esac
	echo 'and press return to load it, or enter q to quit. > \c' >&2
	read ans
	case $ans in
	  q )   exit ;;
	esac
	echo 'Wait.' >&2
	if [ $N = -1 ]
	then
		N=`/etc/memsize $IF`
		N=`expr \( \( $N + 511 \) / 512 + $BS - 1 \) / $BS`
		ulimit `expr $N \* $BS`
	fi
	if [ $COUNT -gt $N ]
	then
		COUNT=$N
	fi
	echo dd if=$IF bs=${BS}b count=$COUNT >&2
	dd if=$IF bs=${BS}b count=$COUNT
	N=`expr $N - $COUNT`
	SKIP=`expr $SKIP + $COUNT`
    done
} > $1

ulimit $OLIM

echo "System dump copied into $1.  Use crash(1M) to analyze the dump."
