#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)backup:backup.sh	1.1.1.1"

#	Backup script 
#	Options:
#	c - complete backup
#	p - incremental ("partial") backup
#	h - backup history
#	u "user1 [user2]" - backup users (use "all" to backup all users)
#	t - backup done to tape; only used if backing up to tape
#	d - special device name; defaults to /dev/rdsk/f0q15d
#	f "<files>" - backup by file name (argument must be in quotes)

USAGE="backup -d "device" [ -t ] [ -c | -p | -u [user] | -f <files> ]\n backup -h"
MEDIA="floppy disk"
CPIO=cpio
CPIOFLGS="-ocB"
DEV=""
BTYPE=""
BACKUP=/etc/Backup
IGNORE=/etc/Ignore
DIR=
trap "cd /; rm -f /tmp/FFILE$$ /tmp/FILE$$ /tmp/VFILE$$ /tmp/flp_index; echo \"\nYou have \
cancelled the Backup\n\"; exit 1 " 1 2 3 9 15

dofind() {
	find $1 $2 -print 2>/dev/null | while read file
	do
	if [ -f $file ]
		then du -a $file
	elif [ -d $file ]
		then echo "D\t$file"
	fi
done >> /tmp/FILE$$
}





while getopts tcphu:f:d: c
do
	case $c in
	c)	type=c
		BTYPE="Complete System"
		;;
	p)	type=p
		BTYPE="Incremental System"
		;;
	d)	DEV="$OPTARG"
		;;
	t)	MEDIA="cartridge tape"
		CPIOFLGS="-oc -C 10240 "
		scsi=`devattr $DEV scsi`
		if [ "$scsi" = "true" ]
		then
			CPIOFLGS="-oc -C 65536 "
		fi
		;;
	h)	type=h;
		;;
	u)	type=u
		user="$OPTARG"
		if [ -n "$FILES" ]
		then
			echo "Error: -u and -f options can't be used together"
			exit 1
		fi
		BTYPE="User"
		;;
	f)	type=f
		FILES="$OPTARG"
		if [ -n "$user" ]
		then
			echo "Error: -u and -f options can't be used together"
			exit 1
		fi
		BTYPE="Selective"
		;;
	\?)	echo "backup: $USAGE"
		exit 0
		;;
	*)	echo "backup: $USAGE"
		exit 1
		;;
	esac
done

case "$type" in

	"")
		echo "backup: $USAGE"
		exit 1
		;;
	h)
		
		if [ -s /etc/.lastbackup ]
		then
			echo "Last complete backup done on `cat /etc/.lastbackup`"
		else
			echo "No complete backup has been done."
		fi
		if [ -s /etc/.lastpartial ]
		then
			echo "Last incremental backup done on `cat /etc/.lastpartial`"
		else
			echo "No incremental backup has been done."
		fi
		exit 0
		;;
esac


shift `expr $OPTIND - 1`

if [ ! "$DEV" ]
then
	if [ "$MEDIA" = "cartridge tape" ]
	then
		DEV="/dev/rmt/c0s0"
	elif [ -c "/dev/rdsk/f1"  ]
	then
		/sbin/flop_num
		if [ $? = 1 ]
		then
			DEV="/dev/rdsk/f0"
		else
			while (true)
			do
				echo "\nThe system has two floppy drives."
				echo "Strike ENTER to backup to drive 0"
				echo "or 1 to backup to drive 1.  \c"
				read ans
				if [ "x$ans" = "x0" -o "x$ans" = "x" ]
				then
					DEV="/dev/rdsk/f0"
					break
				elif [ "x$ans" = "x1" ]
				then
					DEV="/dev/rdsk/f1"
					break
				else
					continue
				fi
			done
		fi
	else
		DEV="/dev/rdsk/f0"
	fi
fi

MESSAGE="You may remove the $MEDIA. To exit, strike 'q' followed by ENTER.
To continue, insert $MEDIA number %d and strike the ENTER key. "

UID=`id | cut -c5`
if [ "$UID" != "0" ]
then
	RNAME=`id | cut -d\( -f2 | cut -d\) -f1`
	if [ "$type" = "c" -o "$type" = "p" ]
	then
		echo "\"$RNAME\" may only select 'Backup History' \
or 'Private Backup'."
		exit 1
	fi
	if [ \( "$type" = u -a "$user" = all \) ]
	then
		echo "\"$RNAME\" may only select 'Backup History' \
or 'Private Backup'."
		exit 1
	fi
fi

case $type in
	c)
#	handle complete backup
		TIR=/
		file2=/etc/.lastbackup
		;;
	p)
#		set up for an incremental update
#		assumption: date IN the file is the same as date OF the file
		if [ ! -s /etc/.lastbackup ]
		then
			echo "A complete backup up must be done before the Incremental backup."
			exit 1
		fi
		if [ ! -s /etc/.lastpartial ]
		then 
			NEWER="-newer /etc/.lastbackup"
		else 
			NEWER="-newer /etc/.lastpartial"
		fi
		file2=/etc/.lastpartial
		TIR=/
		;;
	f)
		DIR="$FILES"
		NEWER=
		TIR=
		;;
	u)
		DIR=
		if [ "$user" = all ]
		then
			user="`pwdmenu`"
			if [ "Name=N O N E" = "$user" ]
			then
			   echo "There are no users to backup"
			   exit 1
			fi
			for i in `pwdmenu`
			do
				T=`echo $i | cut -d= -f2`
				T=`grep "^$T:" /etc/passwd | cut -d: -f6`
				DIR="$DIR $T"
			done
			TIR=/usr
		else
			for i in $user
			do
				TDIR=`grep "^$i:" /etc/passwd | cut -d: -f6`
				if [ -z  "$TDIR" ]
				then
					echo "$i doesn't exist"
					continue
				else
					DIR="$DIR $TDIR"
				fi
			done
			if [ -z "$DIR" ]
			then
				echo "No users to back up."
				exit 1
			fi
			TIR=/usr
		fi
		;;
	*)	# THIS CASE SHOULDN'T BE REACHED
		echo "Invalid type $type"
		echo "backup: $USAGE"
		exit 1
		;;
esac

message -d "Calculating approximate number of $MEDIA(s) required. Please wait."

#  Compute Blocks, number of floppies etc ..
>/tmp/FILE$$
if [ "$type" = "c" -o "$type" = "p" ]
then
	if [ -s "$BACKUP" ]
	then
		> /tmp/FFILE$$
		for file in `cat $BACKUP`
		do
			if [ -f "$file" ]
			then
				du -a $file >>/tmp/FFILE$$
			else
				DIR="$file $DIR"
			fi
		done 
	fi
	if [ -z "$DIR"  -a ! -s /tmp/FFILE$$ ]
	then
		DIR=/
	fi
fi

dofind "$DIR" "$NEWER"

if [ "$type" = "c" -o "$type" = "p" ]
then
	if [ -s "$IGNORE" ]
	then
		cat $IGNORE | while read file
		do
			if [ -n "$file" ]
			then
				grep -v "^[0-9D]*	$file" /tmp/FILE$$ >/tmp/VFILE$$
				mv /tmp/VFILE$$ /tmp/FILE$$
			fi
		done
	fi
	cat /tmp/FFILE$$ >>/tmp/FILE$$ 2>/dev/null
fi

grep -v "^D" /tmp/FILE$$ > /tmp/VFILE$$
cp /tmp/FILE$$ /tmp/flp_index
if [ ! -s /tmp/VFILE$$ ]
then
	if [ "$type" != "u" -a "$type" != "f" ]
	then
		if [ ! -s /etc/.lastpartial ]
		then 
			set `cat -s /etc/.lastbackup`
		else 
			set `cat -s /etc/.lastpartial`
		fi
		message -d "There are no new files since \
the last Incremental Backup on $2 $3.  Therefore no Backup will be done."
	else
		echo  "There are no files to be backed up."
	fi
	rm -f /tmp/FFILE$$ /tmp/FILE$$ /tmp/VFILE$$ /tmp/flp_index
	exit 0
fi

BLOCKS=`cut -f1 < /tmp/VFILE$$ | sed -e '2,$s/$/+/' -e '$s/$/p/'| dc`

# If there are only directories to back up, BLOCKS could be 0.  Make sure
# the message tells the user at least 1 media is needed.

if [ $BLOCKS -eq 0 ]
then
	BLOCKS=1
fi
cut -f2 /tmp/FILE$$ > /tmp/VFILE$$

# Block maxs are:
# 702 for 360K, 1422 for 720K, 2370 for 1.2MB and 2844 for 1.44MB floppies
# 131072 for the 60MB cartridge tape
# 257000 for the 125MB cartridge tape. This also allows for cpio overhead.

if [ "$MEDIA" = "cartridge tape" ]
then
	CTAPEA=`expr \( $BLOCKS / 131072 \) + \( 1 \& \( $BLOCKS % 131072 \) \)`
	CTAPEB=`expr \( $BLOCKS / 257000 \) + \( 1 \& \( $BLOCKS % 257000 \) \)`
	CTAPEC=`expr \( $BLOCKS / 514000 \) + \( 1 \& \( $BLOCKS % 514000 \) \)`
	TIME=`expr ${CTAPEA} \* 30`
	echo /tmp/flp_index > /tmp/FILE$$
	cat /tmp/VFILE$$ >> /tmp/FILE$$ 2>/dev/null
	echo "BLOCKS=$BLOCKS" >> /tmp/flp_index
	rm -f /tmp/VFILE$$
	message "The backup will need approximately:\n\
$CTAPEA cartridge tape(s) for a 60 MB drive or \n\
$CTAPEB cartridge tape(s) for a 125 MB drive \n\
$CTAPEC cartridge tape(s) for a 320 MB drive \n\
and will take no more than $TIME minute(s).\n\n\
Please insert the first cartridge tape.  Be sure to number the cartridge tape(s) \
consecutively in the order they will be inserted."
else
	FP5D9=`expr \( $BLOCKS / 690 \) + \( 1 \& \( $BLOCKS % 690 \) \)`
	FP5H=`expr \( $BLOCKS / 2360 \) + \( 1 \& \( $BLOCKS % 2360 \) \)`
	FP3H=`expr \( $BLOCKS / 2830 \) + \( 1 \& \( $BLOCKS % 2830 \) \)`
	FP3Q=`expr \( $BLOCKS / 1410 \) + \( 1 \& \( $BLOCKS % 1410 \) \)`
	TIME=`expr \( ${FP5D9}0 / 15 \) + \( 1 \& \( ${FP5D9}0 % 15 \) \)`
	echo /tmp/flp_index > /tmp/FILE$$
	cat /tmp/VFILE$$ >> /tmp/FILE$$ 2>/dev/null
	echo "BLOCKS=$BLOCKS" >> /tmp/flp_index
	rm -f /tmp/VFILE$$
	message "The backup will need approximately:\n\
$FP5H formatted 1.2MB 5.25\" floppy disk(s) or\n\
$FP5D9 formatted 360KB 5.25\" floppy disk(s) or\n\
$FP3H formatted 1.44MB 3.5\" floppy disk(s) or\n\
$FP3Q formatted 720KB 3.5\" floppy disk(s).\n\
and will take no more than $TIME minute(s).\n\n\
Please insert the first floppy disk.  The floppy disk(s) you are using \
for the backup MUST be formatted.  Be sure to number the floppy disks \
consecutively in the order they will be inserted."
fi

echo "\n$BTYPE backup in progress\n\n\n"

cat /tmp/FILE$$ | ${CPIO} ${CPIOFLGS} -M"$MESSAGE" -O $DEV
err=$?
if [ "$err" = "4" ] 
then
	message -d "You have cancelled the Backup to $MEDIA(s)."
elif [ "$err" != "0" ] 
then
	if [ "$MEDIA" = "cartridge tape" ]
	then
		message -d "An error was encountered while writing to the ${MEDIA}. \
Please be sure the $MEDIA is inserted properly, and wait for the notification \
before removing it."
	else
		message -d "An error was encountered while writing to the ${MEDIA}. \
Please be sure all your $MEDIA(s) are formatted, and wait for the notification \
before removing them."
	fi
else
	if [ $type = c ]
	then
		message -d "The Complete Backup is now finished."
	elif [ $type = p ]
	then
		set `cat -s /etc/.lastbackup`
		eval M=$2 D=$3
		if [ "$NEWER" = "-newer /etc/.lastbackup" ]
		then 
			set `cat -s  /etc/.lastbackup`
		else 
			set `cat -s  /etc/.lastpartial`
		fi
		message -d  "The incremental backup of the files that have changed \
since the last backup, on $2 $3, is now finished.\n\n\
In order to restore the system totally, first restore the last \
complete backup dated $M $D, and then restore the incremental backup(s) that \
you have performed since $M $D starting with earliest one and \
ending with the one you have just finished."
	else
		message -d "Backup is now completed."
	fi
	if [ "$TIR" = "/" ]
	then
		if [ "$file2" = "/etc/.lastbackup" ]
		then 
			rm -f /etc/.lastpartial
		fi
		date > $file2
	fi
fi
rm -f /tmp/FFILE$$ /tmp/FILE$$ /tmp/VFILE$$ /tmp/flp_index
