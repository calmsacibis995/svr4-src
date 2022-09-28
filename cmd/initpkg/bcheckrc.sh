#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)initpkg:./bcheckrc.sh	1.5.22.2"

if u3b2
then
echo "
#!/sbin/sh
#	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	\"@(#)initpkg:bcheckrc.sh	1.26\"

# This file has those commands necessary to check the file
# system, date, and anything else that should be done before mounting
# the file systems.

rootfs=/dev/root
rrootfs=/dev/rroot
vfstab=/etc/vfstab

# Try to get the file system type for root.  If this fails,
# it will be set to null.  In that case, we hope that vfstab
# is around so that fsck will use it for the type.
set \`/sbin/df -n /\` > /dev/null
if [ \"\$3\" = \"\" ]
then
	fstyp=\"\"
else
	fstyp=\"-F \$3\"
fi

/sbin/fsck \$fstyp -m \${rrootfs}  >/dev/null 2>&1

if [ \$? -ne 0 ]
then
	echo \"The root file system (\${rrootfs}) is being checked.\"
	/sbin/fsck \$fstyp -y \${rrootfs}

	case \$? in
	  0|40)	# remount the root file system
		echo \"  *** ROOT REMOUNTED ***\"
		/sbin/uadmin 4 0
		;;

	  39)	# couldn't fix root - enter a shell
		echo \"  *** ROOT COULD NOT BE REPAIRED - Entering shell\"
		/sbin/sh
		# repair work has hopefully completed so reboot
		echo \"  *** SYSTEM WILL ENTER FIRMWARE AUTOMATICALLY ***\"
		/sbin/fsrootboot
		;;

	  *)	# fsck determined reboot is necessary
		echo \"  *** SYSTEM WILL ENTER FIRMWARE AUTOMATICALLY ***\"
		/sbin/fsrootboot
		;;
	esac
fi

#
#  Comment out for now:
#

# put root into mount table
echo \"\${rootfs} /\" | /sbin/setmnt


# Initialize name of system from name stored in /etc/nodename.
[ -s /etc/nodename ] && read node </etc/nodename && [ -n node ] &&
	/sbin/uname -S \"\$node\"
set \`/sbin/uname -a\`
echo \"Node: \$2\" >&2

/sbin/mount /proc > /dev/null 2>&1

/sbin/mount /dev/fd > /dev/null 2>&1

while read bdevice rdevice mntp fstype fsckpass automnt mntopts
do
	# check for comments
	case \${bdevice} in
	'#'*)	continue
	esac

	# see if this is /stand or /var - check and mount if it is
	if [ \"\${mntp}\" = \"/stand\" -o \"\${mntp}\" = \"/var\" ]
	then
		/sbin/fsck -F \${fstype} -m  \${rdevice}  >/dev/null 2>&1
		if [ \$? -ne 0 ]
		then
			echo \"The \$mntp file system (\${rdevice}) is being checked.\"
			/sbin/fsck -F \${fstype} -y  \${rdevice}
		fi
		/sbin/mount \${mntp} > /dev/null 2>/dev/null
	fi
done < \$vfstab
"> bcheckrc
fi

if i386
then
echo "
#!/sbin/sh
#	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	\"@(#)initpkg:bcheckrc.sh	1.26\"

# This file has those commands necessary to check the file
# system, date, and anything else that should be done before mounting
# the file systems.

/sbin/dumpcheck
echo MARK > /dev/rswap

rootfs=/dev/root
rrootfs=/dev/rroot
vfstab=/etc/vfstab

# Try to get the file system type for root.  If this fails,
# it will be set to null.  In that case, we hope that vfstab
# is around so that fsck will use it for the type.
set \`/sbin/df -n /\` > /dev/null
if [ \"\$3\" = \"\" ]
then
	fstyp=\"\"
else
	fstyp=\"-F \$3\"
fi

/sbin/fsck \$fstyp -m  \${rrootfs}  >/dev/null 2>&1

if [ \$? -ne 0 ]
then
	echo \"The root file system (\${rrootfs}) is being checked.\"
	/sbin/fsck \$fstyp -y \${rrootfs}

	case \$? in
	  0|40)	# remount or reboot the root file system
		echo \"  *** ROOT REMOUNTED ***\"
		uadmin 4 0
		;;

	  39)	# couldn't fix root - enter a shell
		echo \"  *** ROOT COULD NOT BE REPAIRED - Entering shell\"
		/sbin/sh
		# repair work has hopefully completed so reboot
		echo \"  *** SYSTEM WILL REBOOT AUTOMATICALLY ***\"
		uadmin 2 2
		;;

	  *)	# fsck determined reboot is necessary
		echo \"bcheckrc: warning, return value \$? from fsck\"
		echo \"  *** SYSTEM WILL REBOOT ***\"
		uadmin 2 0
	esac
fi

#
#  Comment out for now:
#

# put root into mount table
echo \"\${rootfs} /\" | /sbin/setmnt


# Initialize name of system from name stored in /etc/nodename.
[ -s /etc/nodename ] && read node </etc/nodename && [ -n node ] &&
	/sbin/uname -S \"\$node\"
set \`/sbin/uname -a\`
echo \"Node: \$2\" >&2

/sbin/mount /proc > /dev/null 2>&1

/sbin/mount /dev/fd > /dev/null 2>&1

while read bdevice rdevice mntp fstype fsckpass automnt mntopts
do
	# check for comments
	case \${bdevice} in
	'#'*)	continue
	esac

	# see if this is /var - check and mount if it is
	if [ \"\${mntp}\" = \"/var\" ]
	then
		/sbin/fsck -F \${fstype} -m  \${rdevice}  >/dev/null 2>&1
		if [ \$? -ne 0 ]
		then
			echo \"The \$mntp file system (\${rdevice}) is being checked.\"
			/sbin/fsck -F \${fstype} -y  \${rdevice}
		fi
		/sbin/mount \${mntp} > /dev/null 2>/dev/null
	fi
done < \$vfstab
"> bcheckrc
fi
