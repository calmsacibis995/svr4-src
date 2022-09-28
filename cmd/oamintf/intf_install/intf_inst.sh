#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:intf_install/intf_inst.sh	1.8.2.1"

################################################################################
#
# intf_install:
#
# This script is called during the installation process for a new package.
# It is called by a class action script that operates on the mif class.
# It takes newly installed ".mif" files located in directory
#
# $PKGSAV/intf_install
#
# and applies the changes indicated to the OAM Interface structure.
#
# $OAMBASE is the base OAM directory.  It is set to /usr/sadm/sysadm
# $PKGINST is a unique name indicating the package instance.  It is an
# environment variable set by the install feature and exported so it is
# available to anything executed by the class action script.
#
################################################################################

ERR=0
PROG=$0
OAMBASE=/usr/sadm/sysadm
INTFBASE=$OAMBASE/menu
INSTBIN=/usr/sadm/install/bin
INSTDIR=/var/sadm/pkg/intf_install
export OAMBASE INTFBASE INSTBIN

#  TMPFILE - used to contain info from concatenated & sorted .mif files
#            also used to contain list of tmp.menu files to remove in case
#            installation of interface is discontinued after xqtion of mod_menus
TMPFILE=/tmp/t.$$

#  INST_LOG - installation log file - contains log of changes made by mod_menus
#  INST_FILE - complete path to installation log file
INST_LOG=Il.$$
INST_FILE=/tmp/$INST_LOG

#  EXPR_LOG - express mode log file - contains log of express mode changes to be
#             made to the express file.  Created by mod_menus
#  EXPR_FILE - complete path to express mode log file
EXPR_LOG=Ie.$$
EXPR_FILE=/tmp/$EXPR_LOG

#  COMMFILE - tmp file name of file that contains changes to be COMMITTED
#  COMM_FILE - complete path to temporary file COMMFILE
COMMFILE=c.$$
COMM_FILE=/tmp/$COMMFILE

SUCCESS() {
	echo "## Interface installation for <$PKGINST> completed successfully." >&2
}

ERROR() {
	echo "$PROG:ERROR:$*" >&2
}

commit_chgs() {
	while read tmpfile newfile
	do
		if [ -n "$tmpfile" ] && [ -n "$newfile" ]
		then
			echo $newfile >&2
			echo $newfile e 0644 root sys
			mv $tmpfile $newfile || return 1
		fi
	done | installf -c OAMintf $PKGINST -
	[ $? -ne 0 ] && return 1
	#final installf to indicate that installation is final
	installf -f $PKGINST
	return $?
}

rm_tmpmenus() {
	ret=0
	while read line
	do
		if [ -n "$line" ]
		then
			rm $line || ret=1
		fi
	done
	return $ret
}

find_dups() {
	retcode=0
	opath="null"
	savifs="$IFS"
	IFS="^"
	while read path junk
	do
		if [ -n "$path" ] 
		then
			if [ "$path" = "$opath" ]
			then
				#duplicate path - print error and exit
				ERROR "duplicate path <$path>"
				retcode=1
				break
			fi
			opath=$path
		fi
	done
	IFS="$savifs"
	return $retcode
}

echo "## Modifying interface menus and directories" >&2
rm -f $TMPFILE $INST_FILE $EXPR_FILE $COMM_FILE 

for file in $PKGSAV/intf_install/*.mi
do
	# remove comment lines from .mi files
	grep -v "^#" $file >> $TMPFILE
done

# sort the temporary file
sort -d -o $TMPFILE $TMPFILE || exit 2
find_dups < $TMPFILE || exit 2

# now make modifications to the Interface menu files and directory structure
if $INSTBIN/mod_menus $TMPFILE $INST_FILE $EXPR_FILE
then
	:
else
	ERROR "Errors detected during Interface Menu Modification phase"
	ERR=2
fi

#sort install log file 
sort -d -u -o $INST_FILE $INST_FILE || exit 2

# remove NEWDIR entries - save result in $COMM_FILE
sed -e "/NEWDIR/d" $INST_FILE > $COMM_FILE || exit 2

# extract only first field of COMM_FILE & put in TMPFILE.  This will be list
# of tmp.menu files to delete in case installation is discontinued.
cut -d" " -f1 $COMM_FILE > $TMPFILE || exit 2

if [ $ERR -ne 0 ]
then
	ans=`ckyorn -Q -d yes -p "Do you wish to continue the 
		installation process?" < /dev/tty`
	if [ "$ans" != y ]
	then
		echo "Interface Menu File Changes Will not be Committed" >&2
		rm_tmpmenus < $TMPFILE
		rm -f $TMPFILE $INST_FILE $EXPR_FILE $COMM_FILE 
		exit 3  #user request to stop
	fi
fi


echo "## Committing interface changes." >&2
if commit_chgs < $COMM_FILE 
then
	:
else
	ERROR "Errors detected in committing interface changes."
	ERR=2
fi

echo "## Modifying interface express mode invocation file." >&2
if sort -t\^ +0d -1d +3r -o $EXPR_FILE $EXPR_FILE &&
	$INSTBIN/ie_build $EXPR_FILE
then
	:
else
	ERR=2
	ERROR "Errors detected in interface express mode modifications"
fi

#remove temporary & log files
rm -f $TMPFILE $INST_FILE $EXPR_FILE $COMM_FILE 

echo "## Interface installation for <$PKGINST> \c" >&2
if [ $ERR -eq 0 ]
then
	echo "completed successfully." >&2
	exit 0
else
	echo "partially failed." >&2
	exit 2
fi
