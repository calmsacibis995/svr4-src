#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)filemgmt:bin/mountfs.sh	1.1.3.1"
# mountfs will actually mount the file system for the specified device

for f in fsname mount.err mnt.ok mnt.nok
do
	if test -r /tmp/$f
	then
		rm -f /tmp/$f
	fi
done

#Blatantly borrowed from:

#filemgmt:bin/mountfs.sh	1.3.1.2
#	Mount a removable medium file system.
#help# 
#help#	Mountfsys mounts a file system, found on a removable medium, making
#help#	it available to the user.  The file system is unmounted with the
#help#	"umountfsys" command.  THE MEDIUM MUST NOT BE REMOVED WHILE THE
#help#	FILE SYSTEM IS STILL MOUNTED.

#!	chmod +x ${file}
rc=2
flags="-qq -k$$"	# flags for checkyn to implement [q] (quit)
trap 'exit $rc' 1 2 15

unset disklabelarg makedir mountit readonly patterns

#ddrive=${1}
#ndrive=`/usr/lbin/drivename ${ddrive}`

bdrv=`devattr ${1} bdevice`
cdrv=`devattr ${1} cdevice`
pdrv=`devattr ${1} pathname`
if  [ $bdrv ] 
then ddrive=$bdrv
else if  [ $cdrv ] 
	then ddrive=$cdrv
	else if  [ $pdrv ] 
		then ddrive=$pdrv
		else 	
			echo "   Error - ${1} does not have a device pathname" >>/tmp/mnt.nok
			exit 1
     	     fi
     fi
fi

ndrive="${1} drive"

case "${2}" in
	'read only' )	readonly=-r
			;;
	'*' )		;;
esac

l=`/etc/labelit ${ddrive} 2>/dev/null`
trap "	trap '' 1 2
	echo You may remove the medium from the ${ndrive}. >>/tmp/mnt.nok" 1
eval `/usr/lbin/labelfsname "${l}"`

makedir=
if [ ! -d /${fsname} ]
then
	makedir="
   (There is not currently a /${fsname} directory,
   but this procedure will make one.)
" >>/tmp/mnt.ok 2>&1
fi

trap "" 0 1 2 15
if [ ! -d /${fsname} ]
then
	mkdir /${fsname}
	madedir=yes
fi
if  mountmsg=`/etc/mount ${ddrive} /${fsname} ${readonly} 2>&1`
then
	echo >>/tmp/mnt.ok 2>&1
	echo "   Disk '${label}', file system '/${fsname}' mounted. ${makedir}" >>/tmp/mnt.ok 2>&1
	echo "   DO NOT REMOVE THE MEDIUM UNTIL IT IS UNMOUNTED!" >>/tmp/mnt.ok 2>&1
	rc=0
	exit $rc
fi
if [ ${madedir} ]
then
	rmdir /${fsname}
fi
echo '   The mount failed.' >>/tmp/mnt.nok 2>&1
case "${mountmsg}" in
*'needs checking'* )
	echo '   The file system on this medium needs to be checked.
   Suggestion:  use "check" to look for and repair file system damage.' >>/tmp/mnt.nok 2>&1
	rc=1
	break	;;
*write-protected* )
	echo '   The medium is write protected and cannot be mounted for update.
   Suggestions:  1)  remove the write protection.
     	         2)  mount it for reading only.\n' >>/tmp/mnt.nok 2>&1
	echo '   Mount the file system for reading only and try again.' >>/tmp/mnt.nok 2>&1
	rc=1
	break
;;
* )
	echo "   ${mountmsg}" >>/tmp/mnt.nok 2>&1
	rc=1
	break
esac
echo >>/tmp/mnt.nok 2>&1
echo "   You may remove the medium from the ${ndrive}." >>/tmp/mnt.nok 2>&1
exit $rc
