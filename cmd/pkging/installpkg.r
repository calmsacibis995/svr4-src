#!/sbin/sh
#
#ident	"@(#)pkging:installpkg.r	1.2"
#
# ----------------------------------------------------------------------
# PURPOSE:  Install application software either 3b2 or PLUS style.
# ----------------------------------------------------------------------
# CONTENTS FILE LAYOUT:
# KEY_NAME TYPE NAME
# KEY_NAME is the date+pid+number if needed to make unique
# TYPE is 1 of PLUS stlye and 2 for 3b2 style.
# NAME is the Name file.
PATH=/sbin:/usr/sbin:/etc:/usr/bin
export PATH

trap "trap '' 1 2 3 9 15; cd /; /sbin/umount /install > /dev/null 2>&1; rm -rf ${TMPDIR} ${CPLOG} > /dev/null 2>&1; echo You have canceled the installation.; exit 1" 1 2 3 9 15

INDIR=/usr/lib/installed
CONTENTS=${INDIR}/CONTENTS
TMPDIR=/usr/tmp/install$$

FDMESS="Please insert the floppy disk.\n\nIf the program installation \
requires more than one floppy disk, be sure to insert the disks in the \
proper order, starting with disk number 1.\nAfter the first floppy disk, \
instructions will be provided for inserting the remaining floppy disks."

# Check if root is performing the operation
id | grep "(root)" > /dev/null
if [ "$?" = "0" ]
then
	id | grep "euid=" > /dev/null
	if [ "$?" = "0" ] #Did get root above; no euid string
	then
		id | grep "euid=0(root)" > /dev/null
		if [ "$?" = 0 ]
		then
			UID=0
		else
			UID=1
		fi
	else
		UID=0
	fi
else
	UID=1
fi
if [ "$UID" != 0 ]
then echo "You must be root or super-user to install software."
     exit 1
fi

if [ -z "$TERM" ]
then TERM=AT386-M
fi
CLEAR=`tput clear 2>/dev/null`
unset ROOT

cd /
if [ ! -d /usr/options ]
then
	rm -rf /usr/options
	mkdir /usr/options
	chmod 755 /usr/options
fi
#
# Added Support for Tape Installation
#
if [ -c /dev/rmt/c0s0 -o -c /dev/scsi/qtape1 ]
then
	/sbin/flop_disk
	case "$?" in
	0) break ;;
	1) exec /sbin/sh /usr/lbin/Install.tape
	   echo "Can not install from tape."
	   exit 1 ;;
	*) exit 1 ;;
	esac
fi
#
# end tape support
#
if [ ! -d /install ]
then
	rm -rf /install
	mkdir /install
	chmod 755 /install
fi

cd /
/sbin/mount | grep "^/install" > /dev/null 2>&1
if [ $? = 0 ]
then
	message -d "Cannot run installpkg.  The /install directory is currently \
mounted.  Please unmount /install and then try again."
	exit 1
fi

MES1=
/sbin/flop_num
if [ $? = 2 ]
then
	while true
	do
		echo "\nThis system has two floppy drives.\n\
Strike ENTER to install from drive 0\n\
or 1 to install from drive 1.  \c"
		read ans
		if [ "$ans" = 1 ]
		then
			DRIVE=/dev/dsk/f1
			FDRIVE=/dev/rdsk/f1
			FD=1
			break
		elif [ "$ans" = "" -o "$ans" = 0 ]
		then
			DRIVE=/dev/dsk/f0
			FDRIVE=/dev/rdsk/f0
			FD=0
			break
		fi
	done
else
	DRIVE=/dev/dsk/f0
	FDRIVE=/dev/rdsk/f0
	FD=0
fi
while [ 1 ]
do
	message -c $MES1 $FDMESS
	if [ $? != 0 ]
	then exit 1
	fi
	echo "$CLEAR"
	echo "\n\n\tInstallation is in progress -- do not remove the floppy disk."
	/sbin/mount $DRIVE /install -r > /dev/null 2>&1
	if [ $? = 0 ]
	then
		/sbin/umount /install > /dev/null 2>&1
		break
	fi

	/sbin/mount ${DRIVE}t /install -r > /dev/null 2>&1
	if [ $? = 0 ]
	then
		/sbin/umount /install > /dev/null 2>&1
		DRIVE=${DRIVE}t
		break
	fi

	/sbin/umount /install > /dev/null 2>&1
	seq=`dd if=${DRIVE}t bs=512 count=1 2>/dev/null`
	if [ "20" = `expr "$seq" : "# PaCkAgE DaTaStReAm"` ]
	then 
		DD=diskette`expr ${DRIVE} : ".*\([0-9]\)" + 1`
		message -d "This package is an OA&M style package.  \
To install this package type in:\n\n\t\tpkgadd -d ${DD}\n"
		exit 1
	fi
	dd if=$FDRIVE of=/dev/null bs=15b count=3 > /dev/null 2>&1
	if [ $? = 0 ]
	then
		exec /sbin/sh /usr/lbin/Install.sh $FDRIVE $FD
	fi

	MES1="Cannot determine the 'type' of floppy.\n"
done
/sbin/mount $DRIVE /install -r > /dev/null 2>&1
if [ -f /install/*/pkginfo ]
then
	/sbin/umount /install > /dev/null 2>&1
	DD=diskette`expr ${DRIVE} : ".*\([0-9]\)" + 1`
	message -d "This package is an OA&M style package.  \
To install this package type in:\n\n\t\tpkgadd -d ${DD}\n"
	exit 1
fi
if [ ! -r /install/install/INSTALL ]
then
	echo "Improperly built software removable medium."
	/sbin/umount /install > /dev/null 2>&1
	exit
fi
cd /tmp
trap "trap '' 1 2 3 9 15; cd /; rm -f /tmp/$$*; /sbin/umount /install; exit 1" 1 2 3
cp /install/install/INSTALL $$INSTALL
chmod +x $$INSTALL
/tmp/$$INSTALL $DRIVE /install "`basename ${DRIVE}` drive" ||
	echo 'WARNING: Installation may not have completed properly.'
trap '' 1 2 3
cd /
rm -f $$INSTALL
/sbin/umount /install > /dev/null 2>&1
rm -rf /tmp/$$*
if [ -f /etc/.new_unix ]
then
	sync; sync
	exec /etc/conf/bin/idreboot
fi
exit 0
