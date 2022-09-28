#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)restore:restore.sh	1.3.2.4"

# PURPOSE: Restore ${CPIO} files from floppy disk
# ---------------------------------------------------------------------
#	Options:
#	c - complete restore
#	d - device; defaults to /dev/rdsk/f0
#	t - tape device being used
#	o - overwrite files
#	i - index file

USAGE="restore [ -d <device> ] [ -c ] [ -i ] [ -o ] [ -t ] [pattern [pattern] ...]"
MEDIA="floppy disk"
CPIO=cpio
CPIOFLGS="cik"
BLKSIZE="-B"
DEV=""
PATTERNS=
SFLAG=
overwrite=false

trap "cd /; rm -f /tmp/R.chk$$ /tmp/R.err$$ /tmp/R.ovw$$ /tmp/cplst$$ /tmp/flp_index;\
echo \"You have cancelled the restore.\"; exit 1 " 1 2 3 9 15
type=c
while getopts d:itcos c
do
	case $c in
	c)	type=c
		;;
	d)	DEV="$OPTARG"
		if [ "/dev/rmt" = "`dirname ${DEV}`" ]
		then
			MEDIA="cartridge tape"
			BLKSIZE=" -C 102400 "
		fi
		;;
	t)	MEDIA="cartridge tape"
		BLKSIZE=" -C 102400 "
		;;
	o)	CPIOFLGS="u$CPIOFLGS"
		overwrite=true
		;;
	i)	type=i
		;;
	s)	SFLAG=s
		;;
	\?)	echo "restore: $USAGE" >&2
		exit 0
		;;
	*)	echo "restore: $USAGE" >&2
		exit 1
		;;
	esac
done
CPIOFLGS="$CPIOFLGS $BLKSIZE"
shift `expr $OPTIND - 1`

while [ -n "$1" ]
do
	PATTERNS="\"$1\" $PATTERNS"
	shift
done

#
# If user is root, then raise the ulimit
#

ULIMIT=0
id | grep "uid=0" > /dev/null
if [ $? -eq 0 ]
then
	ULIMIT=`ulimit`
	ulimit 1048571
fi

if [ ! "$DEV" ]
then
	if [ "$MEDIA" = "cartridge tape" ]
	then
		DEV="/dev/rmt/c0s0"
	elif [ -c "/dev/rdsk/f1" ]
	then
		/sbin/flop_num
		if [ $? = 1 ]
		then
			DEV="/dev/rdsk/f0"
		else
			while (true)
			do
				echo "\nThe system has two floppy drives."  >&2
				echo "Strike ENTER to restore from drive 0" >&2
				echo "or 1 to restore from drive 1.  \c"    >&2

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

MESSAGE="You may remove the archive for the input device. To exit, strike 'q' followed
by ENTER.  To continue, insert the next archive and strike the ENTER key. "

case $type in
c )
	if [ "$SFLAG" = s ]
	then
		message -u "Be sure the first archive volume is inserted." >&2
	else
		message -u "Insert the backup archive in the input device." >&2
	fi
	echo "            Restore in progress\n\n" >&2

	trap '' 1 2 3 9 15
	eval $CPIO -dmv${CPIOFLGS} -M \"'$MESSAGE'\" -I "$DEV" $PATTERNS 2>/tmp/R.err$$ | tee /tmp/R.chk$$ 
	err=$?
	trap 1 2 3 9 15
	if [ $err -eq 4 ]
	then
		echo "You have cancelled the restore" >&2

		#
		# lower ulimit to it's original value if it was raised
		#
		
		if [ ${ULIMIT} -ne 0 ]
		then
			ulimit ${ULIMIT}
		fi
		rm -f /tmp/R.chk$$ /tmp/R.err$$
		exit 1
	elif [ $err -ne 0 ]
	then
		if [ "$overwrite" != "true" ]
		then
			# overwrite was not chosen so errors may be of the form
			# cpio: Existing "file" same age or newer
			# grep these lines + the "2688 blocks" line + the
			#    "312 error(s)" line out of R.err$$
			# If there are any lines left then they must be errors
			# This is messy but since cpio reports these as errors,
			# this is the best we can do.
			egrep -v '(same age or newer)|([0-9]* blocks)|([0-9]* error\(s\))' /tmp/R.err$$ > /tmp/R.ovw$$
			ERR=`wc -l /tmp/R.ovw$$ | tr -s " " " " | cut -f2 -d" "`
			if [ $ERR -gt 0 ]
			then
				cat /tmp/R.ovw$$ >&2
				echo "$ERR error(s)\n\n" >&2
				echo "An error was encountered while reading from the input device." >&2
				echo "Please be sure the archive is inserted properly, and wait" >&2
				echo "for notification before removing it." >&2

				#
				# lower ulimit to it's original value if it was raised
				#
				if [ ${ULIMIT} -ne 0 ]
				then
					ulimit ${ULIMIT}
				fi

				rm -f /tmp/R.chk$$ /tmp/R.err$$ /tmp/R.ovw$$
				exit 1
			else
				rm -f /tmp/R.chk$$ /tmp/R.err$$ /tmp/R.ovw$$
				message -d "The restore is now finished." >&2
			fi
		else
			echo "An error was encountered while reading from the input device." >&2
			echo "Please be sure the archive is inserted properly, and wait" >&2
			echo "for notification before removing it." >&2
			#
			# lower ulimit to it's original value if it was raised
			#
			if [ ${ULIMIT} -ne 0 ]
			then
				ulimit ${ULIMIT}
			fi

			rm -f /tmp/R.chk$$ /tmp/R.err$$
			exit 1
		fi
	else
		NOMATCH=
		for i in $PATTERNS
		do
			A=`echo "$i" | sed 's;\";;g
				s;\.;\\\.;g
				s;\*;\.\*;g'`
			NUMERR=`cat /tmp/R.chk$$ | sed -n "\;$A;p" 2>/dev/null | wc -l`
			if [ "$NUMERR" -eq 0 ] 
			then
				NOMATCH="$NOMATCH\n\t$i"
			fi
		done
		if [ -n "$NOMATCH" ]
		then
			echo "The following files and directories were not found.\n${NOMATCH}\n\n" | pg # >&2
		fi
		rm -f /tmp/R.chk$$ /tmp/R.err$$
		message -d "The restore is now finished." >&2
	fi

	#
	# lower ulimit to it's original value if it was raised
	#

	if [ ${ULIMIT} -ne 0 ]
	then
		ulimit ${ULIMIT}
	fi
	exit
	;;
i)	
	message -c "Insert the backup archive into the input device." >&2
	if [ "$?" -ne 0 ]
	then
		echo "You have cancelled the restore" >&2

		#
		# lower ulimit to it's original value if it was raised
		#
	
		if [ ${ULIMIT} -ne 0 ]
		then
			ulimit ${ULIMIT}
		fi
		exit 1
	fi
	message -d "Reading the backup archive.\nPlease do not remove." >&2
	trap '' 1 2 3 9 15
	xtract "${CPIOFLGS}" /tmp/flp_index  $DEV > /tmp/cplst$$
	cat /tmp/flp_index | sed '/^BLOCK/d' 
	rm -f /tmp/cplst$$ /tmp/flp_index
	;;
*)	echo "Invalid type $type" >&2
	echo "restore: $USAGE" >&2
	#
	# lower ulimit to it's original value if it was raised
	#

	if [ ${ULIMIT} -ne 0 ]
	then
		ulimit ${ULIMIT}
	fi
	exit 1
	;;
esac

#
# lower ulimit to it's original value if it was raised
#

if [ ${ULIMIT} -ne 0 ]
then
	ulimit ${ULIMIT}
fi
