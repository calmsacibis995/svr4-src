#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)initpkg:dumpsave.sh	1.1.4.2"

cat <<\END >dumpsave
#!/sbin/sh

#	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# save a system memory image dump from /dev/rswap to tape or floppy

#ident	"@(#)initpkg:dumpsave.sh	1.22"

# shell variables used:
#
#       DEV     "f" = floppy; "t" = tape
#
#       OF      output device file used by dd
#       BS      block size used by dd, in 512 byte units
#       COUNT   number of blocks to be copied by dd
#       SKIP    number of blocks for dd to skip
#
#       NB      number of BS size blocks on tape/disk
#       N       number of BS size blocks of memory to copy
#

while :
do
	echo '\nDo you want to save it on:'
	echo '  1 - low density 5.25" (360K) diskettes'
	echo '  2 - high density 5.25" (1.2M) diskettes'
	echo '  3 - low density 3.5" (720K) diskettes'
	echo '  4 - high density 3.5" (1.44M) diskettes'
	if [ -w /dev/rmt/c0s0 ]
	then
		echo '  t - Cartridge tape'
	fi
	if [ -w /dev/scsi/qtape1 ]
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
	  t )   OF=/dev/rmt/c0s0     BS=32 NB=10000  DEV=t ;
		if [ -w $OF ]; then break; fi ;;
	  s )   OF=/dev/scsi/qtape1     BS=32 NB=10000  DEV=t ;
		if [ -w $OF ]; then break; fi ;;
	  n )   exit ;;
	esac
	echo '???'
done

if [ "$DEV" = "f" ]
then
/sbin/flop_num
if [ $? = 2 ]
then
	while [ 1 ]
	do
		echo "\nThis system has two floppy drives.\n\
Strike ENTER to save the dump in drive 0\n\
or 1 to save the dump in drive 1.  \c"
		read ans
		if [ "$ans" = 1 ]
		then
			OF=/dev/rdsk/f1t
			break
		elif [ "$ans" = "" -o "$ans" = 0 ]
		then
			OF=/dev/rdsk/f0t
			break
		fi
	done
else
	OF=/dev/rdsk/f0t
fi
fi

# while :
# do
#	echo 'How many megabytes of memory image do you want to save?'
#	echo 'Enter decimal integer or "q" to quit. > \c'
#	read ans
#	case $ans in
#	  q )   exit 0 ;;
#	esac
#	N=`expr \( $ans \* 2048 + $BS - 1 \) / $BS`
#	case $? in
#	  0 )   break;;
#	esac
#	echo '???'
# done

SKIP=0
COUNT=$NB
N=`/etc/memsize /dev/rswap`
N=`expr \( \( $N + 511 \) / 512 + $BS - 1 \) / $BS`

while [ $N -gt 0 ]
do
	if [ $COUNT -gt $N ]
	then
		COUNT=$N
	fi
	echo 'Insert \c'
	case $DEV in
	  f )   echo 'diskette \c' ;;
	  t )   echo 'tape cartridge \c' ;;
	esac
	echo 'and press return, or enter q to quit. > \c'
	read ans
	case $ans in
	  q )   exit 0 ;;
	esac
	echo 'Wait.'
	echo dd if=/dev/rswap of=$OF bs=${BS}b count=$COUNT skip=$SKIP
	dd if=/dev/rswap of=$OF bs=${BS}b count=$COUNT skip=$SKIP
	if [ $? != 0 ]
	then if [ "$DEV" = "t" ]
	     then echo "\nCannot write to this tape.  Please make it write enabled.\n"
	     else echo "\nCannot write to this diskette.  The diskette is either\n\
write protected or not formatted.  Please correct the problem.\n"
	     fi
	     continue
	fi
	N=`expr $N - $COUNT`
	SKIP=`expr $SKIP + $COUNT`
done
echo '\n\nDone.  Use /etc/ldsysdump to copy dump from tape or diskettes'
echo 'Press return to continue >\c'
read ans
END
