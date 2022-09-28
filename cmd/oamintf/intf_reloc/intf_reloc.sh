#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:intf_reloc/intf_reloc.sh	1.2.2.1"
################################################################################
#
# intf_reloc - shell script that relocates menu items from selected
#		menus in pre-SVR4.0 (/usr/admin/menu) simple admin
#		to 4.0 menu structure.
#		To find out what menus get relocated, see static table
#		in reloc.c.  A future enhancement would be to remove
#		the static table from reloc.c and read in the info
#		from a file, either through this shell script or
#		through the C program.
#
################################################################################
ERRMSG1="$0: ERROR: menu relocation did not complete successfully"
ERRMSG2="$0: ERROR: unable to modify express mode invocation file;
    menus are relocated, but may not be accessed via express mode"

ERR=0
OAMBASE=/usr/sadm/sysadm
PKGINST=_PRE4.0
INSTBIN=/usr/sadm/install/bin
TMPFILE=/tmp/intf_reloc_log
export OAMBASE PKGINST

# relocate 3.0 menus to 4.0 menus
echo "## Relocating pre-SVR4 menus to administrative menu structure" >&2

if $INSTBIN/reloc_menus $TMPFILE
then
	echo "## Modifying Express Mode Invocation File"
	sort -t\^ +0d -1d +3r -o $TMPFILE $TMPFILE || ERR=2

	#now modify express mode invocation file
	if $INSTBIN/ie_build $TMPFILE
	then
		:
	else
		ERR=2
		echo "ERRMSG2" >&2
	fi
else
	echo "ERRMSG1" >&2
fi

#now remove temporary file
rm $TMPFILE

exit $ERR
