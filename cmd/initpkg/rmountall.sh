#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)initpkg:./rmountall.sh	1.7.13.1"

if u3b2 || i386
then echo "#!/sbin/sh

#	Mount remote resources according to a file system table
#	such as /etc/vfstab.

#	file-system-table format:
#
#	column 1	block special file name of file system
#	column 2	the file system name to be checked by fsck
#	column 3	mount-point directory
#	column 4	file system type (may be column 4)
#	column 5	the option to check the file system
#	column 6	the option for automount
#	column 7	mount flags (rw, ro, etc.)
#	White-space separates columns.
#	Lines beginning with \"#\" are comments.  Empty lines are ignored.
#	a '-' in any field is a no-op.

#!	chmod +x \${file}

if [ \$# -lt 1 ]
then
	echo >&2 \"Usage:  \$0 file-system-table ...\"
	exit 1
fi

cat \$*  |
	while  read spec fsckdev mountp fstyp fsckpass automnt mntflg
	do
		case \${spec} in
		'#'* | '')	#  Ignore comments, empty lines
			continue
		esac
		case \${spec} in
		'-')		#  Ignore no-action lines
			continue
		esac 
		if [ \"\${fstyp}\" = \"rfs\" -a \"\${automnt}\" = \"yes\" ]
		then
			if [ \"\${mntflg}\" = \"-\" ]
			then	mntflg=rw
			fi
			/usr/sbin/rmount -F rfs -o \${mntflg} \${spec} \${mountp} \\
			>/dev/console 2>&1
		fi

	done
	/etc/rfs/rmnttry&
" >rmountall
fi
