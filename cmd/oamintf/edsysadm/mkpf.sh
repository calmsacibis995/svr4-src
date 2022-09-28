#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:edsysadm/mkpf.sh	1.4.2.1"

################################################################################
#	Module Name: mkpf
#
#	Inputs: 
#		$1 -> prototype file from package description file
#		$2 -> flag to indicate menu/task and change/add
#			- chgmenu
#			- chgtask
#			- addmenu
#			- addtask
#			- mifile
#		$3 -> name of package description file
#		$4 -> location:name
#		$5 -> help file
#		$6 -> comma separated list of task files (Task only).
#
#	Outputs:
#
# 	A prototype file will be created and an entry for each file will 
#	be added to it in the following format:
#	
#		ftype 	 class 	  pathname      mode     owner   group
#		------------------------------------------------------
#   example  ->	  f     OAMadmin   /path        755       root    sys
#
#	ftype ->    a one character field which indicates the file type.
#
#	class ->    the installation class to which the file belongs.  This
#	            name may contain only alphanumeric characters and be no
#		    longer than 12 characters.  The default class is none.
#
#	pathname -> the pathname where the file will reside on the target
#		    machine.  Relative pathnames (those that do not begin
#		    with a slash) indicate that the file is relocatable.
#		    The pathname may be in the form path1=path2 where
#		    path1 indicates the location on the target machine and
#		    path2 indicates the location on the host machine.  This
#		    will be the form this command will create prototype
#		    file with.
#	
#	mode ->     the octal mode of the file.
#
#	owner ->    the owner of the file .
#
#	group ->    the group which the file belongs.
# 	UNIX comands:
#		pwd, rm, basename, dirname, echo, date, cat,
#		cut, grep, sed, expr
################################################################################

# Set path name to temp files
TMP_PFILE=/tmp/$$tmp_pfile	# Temp prototype file
HOLD_PFILE=/tmp/$$hld_pfile	# Hold prototype file
DUP_PFILE=/tmp/${VPID}.dpf	# Temp duplicate file
ADD_PFILE=/tmp/${VPID}.adup     # 
MK_PROTO=/tmp/$$mkproto 	# Temp prototype file for 'mkproto'

# tools bins
EDBIN=$OAMBASE/edbin

#  executables target directory
EXEC_BIN=\$OAMBASE/add-ons/\$PKGINST/bin

# Identification line for prototype file created by edsysadm
TITLE="# Prototype file created by edsysadm on"

# Exit Values
SUCCESS=0
INVALID=1	# other error condition
DUP_MI=2	# duplicate menu information file
TOO_MANY=4	# more than one menu information file
FILE_NFND=5	# file not found
BAD_PROTO=6	# prototype file not created by edsysadm
BAD_LOC=7	# bad location given
OBJ_NFND=8	# menu or task not found in package description file
DUP_PROTO=127	# duplicate prototype file ENTRIES
EXIT_VAL=0

# options for prototype entries
FTYPE=f		# File type
CLASS=OAMadmin	# Class admin
MODE=0755	# Directory mode
OWN=root	# Owner mode
GRP=sys		# Group mode
DEFAULT="!default 0644 root sys"

# Remove temp files
rm -f $TMP_PFILE
rm -f $HOLD_PFILE
rm -f $DUP_PFILE
rm -f $ADD_PFILE

# Assign arguments to descriptive variables
PROTOTYPE="$1"	# prototype file
FLAG="$2"	# File flag
PKGDESC="$3"	# Package description file
LOCATION="$4"	# Logical path location
HELP="$5"	# Help file 
FILES="$6"	# Comma separated file list (includes action file)

################################################################################
#	Module Name: mk_dirs
#
#	Description:
#		Parse TARGET and create prototype entry for each directory
################################################################################
mk_dirs() {

LOC=$TARGET

while [ "`basename $LOC`" != "add-ons" ]
do
	if [ -w "$PROTOTYPE" ]
	then
		grep " $LOC " $PROTOTYPE > /dev/null

		if [ $? -eq 0 ]
		then
			LOC=`dirname $LOC`
		else	
			echo "x $CLASS $LOC $MODE $OWN $GRP" >> $TMP_PFILE
			LOC=`dirname $LOC`
		fi
	else
		echo "x $CLASS $LOC $MODE $OWN $GRP" >> $TMP_PFILE
		LOC=`dirname $LOC`
	fi
done
}

################################################################################
#	Module Name: do_help
#
#	Description:
#		Create a prototype entry for help directory and file
################################################################################
do_help() {

if [ -w "$PROTOTYPE" ]
then
	grep " $TARGET/HELP " $PROTOTYPE > /dev/null ||
		 echo "x $CLASS $TARGET/HELP $MODE $OWN $GRP" >> $TMP_PFILE
else
	echo "x $CLASS $TARGET/HELP $MODE $OWN $GRP" >> $TMP_PFILE
fi
		
# Check if help file is absolute or relative path
#if expr "$HELP" : '^[^/]' > /dev/null
#then
#	echo "$FTYPE $CLASS $TARGET/HELP/`basename $HELP`=`pwd`/$HELP" >> $TMP_PFILE
#else
#	echo "$FTYPE $CLASS $TARGET/HELP/`basename $HELP`=$HELP" >> $TMP_PFILE
#fi

echo "$FTYPE $CLASS $TARGET/HELP/`basename $HELP`=$HELP" >> $TMP_PFILE

}

################################################################################
#	Module Name: do_files
#
#	Description:
#		Create a prototype entry for each file and store in a
#		temporary file.  If a directory is given call pkgproto
#		to generate a prototype file and then manipulate the
#		file before writing it to the final edsysadm prototype
#		file.
################################################################################
do_files() {


files=`echo $FILES | sed -e 's/,/ /g'`
for file in $files
do
	
	# If file is current directory or '.' then
	# assign current directory to SRC.

	if [ "$file" = "`pwd`" ] || [ "$file" = "." ]
	then
		SRC=`pwd`

	# If file is a relative path then assign FILE_NAME the last node
	# ( ex. aaa/bbb/ccc -> FILE_NAME=ccc ) and assign SRC the relative
	#  path prefixed with current directory.

	elif expr "$file" : '^[^/]' > /dev/null 
	then
		FILE_NAME=`basename $file`
#		SRC=`pwd`/$file
		SRC=$file


	# If file is an absolute path then assign FILE_NAME the last node
	# ( ex. aaa/bbb/ccc -> FILE_NAME=ccc ) and assign SRC the absolute
	# path.
		
	else
		FILE_NAME="`expr \"$file\" : '.*/\([^  	]*\)'`"
		SRC=$file
	fi

	#   Check if argument is a directory.  If it is, then
	#   call 'pkgproto' command passing it the source
	#   directory and the target directory and append the
	#   output to a temporary prototype file

	if [ -d "$SRC" ]
	then
		pkgproto -c $CLASS $SRC=$TARGET |
		while read tmp_pline
		do
			dest_path=`echo $tmp_pline |
			sed -e 's/[0-9]* *[^ ]* *[^ ]* *\([^ =]*\).*/\1/'`

			if expr "$tmp_pline" : '^[d]' > /dev/null
			then
				echo $tmp_pline | cut -f1-4 -d" " |
				sed -e 's/^d/x/' \
	    			    -e "s/$/ $OWN $GRP/" \
				>> $TMP_PFILE

			elif [ -x "$dest_path" ]
			then
				echo $tmp_pline | cut -f1-3 -d" " |
				sed -e 's/[0-9]* *[^ ]* *[^ ]* *\([^ =]*\).*/\1/'
				$EXEC_BIN/$FILE_NAME
				>> $TMP_PFILE

 			elif [ "`basename $dest_path`" = prototype ] ||
 			     [ "`basename $dest_path`" = *.mi ]
			then
				continue
 				
			else
				echo $tmp_pline | cut -f1-3 -d" " >> $TMP_PFILE
			fi
		 done

	# Check if file is a regular file

	elif [ -r $SRC ]
	then
		echo "$FTYPE $CLASS $TARGET/$FILE_NAME=$SRC" >> $TMP_PFILE

	# executable file? then apppend entry with special target directory

	elif [ -x "$SRC" ]
	then
		echo "$FTYPE $CLASS $EXEC_BIN/$FILE_NAME=$SRC" >> $TMP_PFILE

	fi
done

}
################################################################################
#
#   Do an entry-by-entry check of the temp prototype file against the existing
#   prtototype file for duplicate entries.  For each duplicate entry found echo
#   it to a file labeled dup_pfile in the current directory, set an exit flag
#   and continue processing.  The contents of the dup_pfile will be displayed
#   to the user after this the function is exited.
#
#	4 files will be manipulated they are:
#		1) prototype - This the actual prototype file.
#		2) dup_pfile - This file contains duplicate entries that
#			       were flag when attempting to be added to the
#		 	       prototype file
#		3) tmp_pfile - All arguments passed will have an entry in
#			       this file.  Each entry will be checked against
#			       the prototype file for dupliacates entries and
#			       also will be edited for correct file type,
#			       class, and owner and group modes.
#		4) hold_pfile - Temporary file to hold valid  entries
################################################################################
chk_dups() {
if [ -w $PROTOTYPE ]
then
	while read ftype class path junk
	do
		npath=`expr "$path" : '\([^=]*\).*'`
		if dups=`grep '^[^ ] [^ ]* '$npath'[= ]' $PROTOTYPE`
		then
			# duplicate
			echo $dups >> $DUP_PFILE
			echo $dups >> $ADD_PFILE
		else
			# not duplicate
			echo $ftype $class $path $junk >> $HOLD_PFILE
			echo $dups >> $ADD_PFILE
		fi
	done < $TMP_PFILE

	[ -r $DUP_PFILE ] && EXIT_VAL=$DUP_PROTO
fi
}
	
################################################################################
#	Module Name: write_proto 
#
#	Description:
#	
################################################################################
write_proto() {
if [ -w $PROTOTYPE ]
then
 	if [ -r $HOLD_PFILE ]
	then
		 cat $HOLD_PFILE >> $PROTOTYPE
	fi

else
	#######################################################
	#
	#   Create a prototype file with comment line indicating
	#   it was created by edsysadm, include date stamp.
	#   
	#   This line will be checked when a prototype file
	#   exist in a directory to assure that it is a 
	#   edsysadm prototype file.
	#
	#######################################################

	echo "$TITLE `date`" > $PROTOTYPE
	echo "$DEFAULT" >> $PROTOTYPE
	cat $TMP_PFILE >> $PROTOTYPE

fi
}

################################################################################
#	Module Name: cleanup 
#
#	Description:
#		Remove temporary files.
#	
################################################################################
cleanup() {

#   Remove temp files
rm -f $HOLD_PFILE
rm -f $TMP_PFILE
rm -f $MK_PROTO

if [ ! -r $DUP_PFILE ]
then
	rm -f $ADD_PFILE
fi

#[ -r $DUP_PFILE ] || rm -f $ADD_PFILE

exit $EXIT_VAL
}

################################################################################
#	MAIN PROCESSING
################################################################################

# Check if prototype file was created by edsysadm.
# If not then exit function a tell user that the 
# prototype file in the current directory cannot be
# altered by the edsysadm command.
# If edsysadm title line not found then exit program

if [ -w $PROTOTYPE ]
then
	grep "$TITLE" $PROTOTYPE >/dev/null || exit $BAD_PROTO
fi

# Don't need to set TARGET if flag is "mifile"
if [ "$FLAG" != "mifile" ]
then
	# Convert logical location to physical target path
	TARGET=`$EDBIN/findmenu -p $LOCATION`
	TARGET=`echo $TARGET | sed 's/\/menu\//\/add-ons\/\$PKGINST\//g'`
fi

# Process prototype 
case $FLAG in
	mifile) 
		echo "x OAMmif $LOCATION $MODE $OWN $GRP" > $TMP_PFILE
		echo "v OAMmif $LOCATION/`basename $FILES`=$FILES" >> $TMP_PFILE
		chk_dups
		write_proto
		cleanup
		;;

	chgmenu)
		mk_dirs
		chk_dups
		write_proto
		cleanup
		;;

	addmenu) 
		mk_dirs
		do_help
		chk_dups
		write_proto
		cleanup
		;;

	chgtask)
		mk_dirs
		do_files
		chk_dups
		write_proto
		cleanup
		;;

	addtask)
		mk_dirs
		do_help
		do_files
		chk_dups
		write_proto
		cleanup
		;;
	*)
		echo "Unknown flag -> $FLAG"
		exit $INVALID
		;;
esac
