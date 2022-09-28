#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)filemgmt:bin/rmdisk.sh	1.1.3.1"
trap 'rm -f /tmp/$$.*' 0

ddrive=$1
if [ "${ddrive}" = "" ]
then
	exit 1
fi
ndrive=`drivename ${ddrive}`

rootdev=`/etc/devnm /`
rootdev=`expr "${rootdev}" : '\(.*\) .*'`
root_six=`expr "${rootdev}" : '\(.*\).'`
root_six="${root_six}6"
ls_root=`ls -l ${root_six}`
root_maj=`expr "${ls_root}" : '.* [ 	*].* [ 	*].* [ 	*]\([0-9][0-9]*\),.*'`
root_min=`expr "${ls_root}" : '.*, *\([0-9][0-9]*\)'`
ls_select=`ls -l ${ddrive}`
sel_maj=`expr "${ls_select}" : '.* [ 	*].* [ 	*].* [ 	*]\([0-9][0-9]*\),.*'`
sel_min=`expr "${ls_select}" : '.*, *\([0-9][0-9]*\)'`
if [ ${root_maj} -eq ${sel_maj} -a ${root_min} -eq ${sel_min} ]
then
	echo "
The ${ndrive} contains the root file system. It may not be removed." >&2
	exit 1
fi

dskname=`samedev ${ddrive} /dev/dsk/c*d*s6`
dskname=`expr "${dskname}" : '.*/\([^/]*\).$'`
havet=`expr "${dskname}" : '.*\(t\).*'`
if [ -n "${havet}" ]
then
  eval `basename ${dskname} | sed -e 's:^c\([0-9]*\)t\([0-9]*\)d\([0-9]*\).*$:slot=\1 tc=\2 drv=\3:'`
  echo "\nRemoving ${ndrive} from configuration (hardware slot ${slot},"
  echo "target controller ${tc}, drive ${drv})."
else
  eval `basename ${dskname} | sed -e 's:^c\([0-9]*\)d\([0-9]*\).*$:slot=\1 drv=\2:'`
  slot=`expr ${slot} - 1`
echo "\nRemoving ${ndrive} from configuration (hardware slot ${slot} drive ${drv})."
fi

> /tmp/$$.lose
> /tmp/$$.keep

exec 3<&0 < /etc/fstab # Redirect stdin to make "exit" work within while

while read dev dir ronly junk
do
	case "${dev}" in
	/dev/dsk/${dskname}*)
		if [ "${dir}" = /usr ]
		then
			echo "
The ${ndrive} contains the /usr file system. It may not be removed." >&2
			exit 1
		fi
		echo "${dev} ${dir} ${ronly} ${junk}" >> /tmp/$$.lose
		;;
	*)
		echo "${dev} ${dir} ${ronly} ${junk}" >> /tmp/$$.keep
		;;
	esac
done

exec <&3 3<&- # Restore stdin, close temporary fd.

if [ -s /tmp/$$.lose ]
then
	echo "\nThis will DESTROY the contents of the following filesystems:\n"
	while read dev dir ronly junk
	do
		if [ "${ronly}" = '-r' ]
		then
			echo "\t${dir} (${dev}) [read-only]"
		else
			echo "\t${dir} (${dev})"
		fi
	done < /tmp/$$.lose
	echo
	if chkyn -k $$ -h '?' -H "
	This will DESTROY the contents of the listed filesystems; all of
	their data will no longer be accessible." \
	-f "Continue?"
	then
		:
	else
		echo "\nAborted."
		exec sh ${myname} ${args}
	fi
fi

> /tmp/$$.umnt

sort +1r -o /tmp/$$.lose /tmp/$$.lose

exec 3<&0 < /tmp/$$.lose # Avoid "while" redirection so that exit works

while read dev dir ronly junk
do
	if /etc/mount | grep "^[^ ]* on ${dev} " > /dev/null
	then
		if /etc/umount ${dev}
		then
			echo "${dev} ${dir} ${ronly} ${junk}" >> /tmp/$$.umnt
			rmdir ${dir}
		else
			echo "\nCannot unmount ${dev} from ${dir}." >&2
			sort +1 /tmp/$$.umnt | while read dev dir ronly junk
			do
				(umask 022 ; mkdir ${dir})
				/etc/mount ${dev} ${dir} ${ronly}
			done
			exec sh ${myname} ${args} <&3 3<&-
		fi
	fi
done

exec <&3 3<&- # Restore stdin, close temporary fd 

cp /etc/fstab /etc/Ofstab
cp /tmp/$$.keep /etc/fstab

eval rm /dev/dsk/${dskname}? /dev/SA/$1 \
	/dev/rdsk/${dskname}? /dev/rSA/$1

echo "
The ${ndrive} (hardware slot ${slot} drive ${drv}) has been
removed from the configuration."

exec sh ${myname} ${args}
