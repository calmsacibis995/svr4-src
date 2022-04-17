#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)acct:dodisk.sh	1.13.3.1"
# 'perform disk accounting'
PATH=:/usr/lib/acct:/usr/bin:/usr/sbin
export PATH
if [ -f  /usr/bin/uts -a /usr/bin/uts ]
then
	DEVLIST=/etc/checklist
	format="dev mnt type comment"
else 
	DEVLIST=/etc/vfstab
	format="special dev mnt fstype fsckpass automnt mntflags"
fi
_dir=/var/adm
_pickup=acct/nite
set -- `getopt o $*`
if [ $? -ne 0 ]
then
	echo "Usage: $0 [ -o ] [ filesystem ... ]"
	exit 1
fi
for i in $*; do
	case $i in
	-o)	SLOW=1; shift;;
	--)	shift; break;;
	esac
done

cd ${_dir}

if [ "$SLOW" = "" ]
then
	if [ $# -lt 1 ]
	then
		while :
		do
			if read $format
		       	then
				if [ "$fsckpass" -ne 1 ]
				then
					continue
				fi
				if [ `expr $dev : '\(.\)'` = \# ]
		       		then
		               		continue
		        	fi
				if [ -f  /usr/bin/uts -a /usr/bin/uts ]
				then
					if [ "$type" = u370 ]
					then
# special uts file system type
# u370diskusg not included in 3b2 accounting pkg
#						u370diskusg $dev > `basename $mnt`.dtmp &
						continue
					else
						echo diskusg $dev > `basename $mnt`.dtmp &
					fi
				else
					if [ "$fstype" != s5 ]
					then
						find $mnt -print | acctdusg > `basename $dev`.dtmp &
					else
						diskusg $dev > `basename $dev`.dtmp &
					fi
				fi
			else
				wait
				break
			fi
		done < $DEVLIST
		cat *.dtmp | diskusg -s > dtmp
		rm -f *.dtmp
	else
		args="$*"
		for i in $args; do
			fstype=`fstyp $i`
			if [ "$fstype" = s5 ]
			then
				s5="$s5 $i"
			else
				mnt=`df -n $i 2>/dev/null | cut -f 1 -d " "`
				if [ -n "$mnt" ]
				then
					other="$other $mnt"
				else
					dirname=`dirname $i`
					if [ "$dirname" = "/dev/rdsk" ]
					then
						basename=`basename $i`
						mnt=`df -n "/dev/dsk/"${basename} 2>/dev/null | cut -f 1 -d " "`
					fi
					if [ -n "$mnt" ]
					then
						other="$other $mnt"
					else
						echo "dodisk: $i not done.  Bad name or not mounted."
					fi
				fi
			fi
		done
		if [ -n "$s5" ]
		then
			diskusg $s5 > dtmp1 &
		fi
		if [ -n "$other" ]
		then
			find $other -print | acctdusg > dtmp2 &
		fi
		wait
		if [ -n "$s5" ]
		then	if [ -n "$other" ]
			then
				cat dtmp[12] | diskusg -s > dtmp
				rm dtmp[12]
			else
				mv dtmp1 dtmp
			fi
		else	if [ -n "$other" ]
			then
				mv dtmp2 dtmp 
			fi
		fi
	fi
else
	if [ $# -lt 1 ]
	then
		args="/"
	else
		args="$*"
	fi
	for i in $args; do
		if [ ! -d $i ]
		then
			echo "$0: $i is not a directory -- ignored"
		else
			dir="$i $dir"
		fi
	done
	if [ "$dir" = "" ]
	then
		echo "$0: No data"
		> dtmp
	else
		find $dir -print | acctdusg > dtmp
	fi
fi

sort +0n +1 dtmp | acctdisk > ${_pickup}/disktacct
chmod 644 ${_pickup}/disktacct
chown adm ${_pickup}/disktacct
