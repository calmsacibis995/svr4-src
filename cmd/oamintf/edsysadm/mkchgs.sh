#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:edsysadm/mkchgs.sh	1.11.3.1"
###########################################################
#	mkchgs
#	arguments are :
#	       Flag Name Description Location Help_file Action Task_files
#
# mkchgs flag name desc locn help actn task,task,task...
#
###########################################################

EXITCODE=0

# Set system calls
CUT=/usr/bin/cut
GREP=/usr/bin/grep
MKDIR=/usr/bin/mkdir
RM=/usr/bin/rm 
CHOWN=/usr/bin/chown
CHGRP=/usr/bin/chgrp
CAT=/usr/bin/cat
FIND=/usr/bin/find
CPIO=/usr/bin/cpio
ECHO=/usr/bin/echo
SORT=/usr/bin/sort
SED=/usr/bin/sed
#

# Return function to include proper exit code and temporary files removal
cleanup() {

# Remove the temp files created by mod_menus if created and error exits.
if [ -s /tmp/rmmodtmp ]
then
	for i in `$CAT /tmp/rmmodtmp`
	do
		$RM -f $i
	done
fi

$RM -f $MI_FILE 2>/dev/null
$RM -f $log_file 2>/dev/null
$RM -f $log_2 2>/dev/null
$RM -f $expr_log 2>/dev/null

exit $EXITCODE
}

# Copy module - copies task files
copy_tasks() {
# Copy action file as well as task files - included in temp file.

# List files and if non-existant error out
for i in `$CAT /tmp/cptasks`
do
	THIS=`$ECHO ${i} | $CUT -f1 -d"="`
	THAT=`$ECHO ${i} | $CUT -f2 -d"="`

	if [ -z "$THAT" ] && [ ${THIS} = "TASK" ]
	then
		continue
	fi
	ls ${THAT} 2> /dev/null || {
	EXITCODE=3
	$RM -f /tmp/cptasks 2>/dev/null
	cleanup
	}
done 2> /dev/null

# Find and cpio files
for i in `$CAT /tmp/cptasks`
do
	THIS=`$ECHO ${i} | $CUT -f1 -d"="`
	THAT=`$ECHO ${i} | $CUT -f2 -d"="`

	if [ -z "$THAT" ] && [ ${THIS} = "TASK" ]
	then
		continue
	fi

	if [ ${THIS} = "TASK" ] 
	then
		fmliobj=`$ECHO ${THAT} | $CUT -f1 -d"."`
		if [ $fmliobj = "Form" ] || [ $fmliobj = "Menu" ] || [ $fmliobj = "Text" ]
		then
			$FIND $THAT -print | $CPIO -pdum $newdir
		else
			# put into the bin directory
			$FIND $THAT -print | $CPIO -pdum $binpath
		fi
	elif [ ${THIS} = "HELP" ]
	then
		cp $THAT $newdir/Help
	else
		$FIND $THAT -print | $CPIO -pdum $newdir
	fi
done 2>/dev/null
	

# If return from find and cpio is error, remove new files and error out
if [ $? -ne 0 ]
then
	for i in `$CAT /tmp/cptasks | $CUT -f2 -d"="`
	do
		$RM $newdir/$i 2>/dev/null
	done
	EXITCODE=3
fi

# remove temp file of action, task files
$RM -f /tmp/cptasks 2>/dev/null

}

###########################################################
# mkchgs flag name desc locn help actn task,task,task...
# main function
#	Set pkginst to _ONLINE
#	Collision Detection
#	Menu Info File Generation
#	Copy Task Files
#	Modify Menus
#	Modify Express Mode Lookup File
#	Commit Changes
###########################################################

# Set Pkginst Variable to _ONLINE
PKGINST=_ONLINE
export PKGINST

ONLINE="online"

# assign variables to arguments
FLAG=${1}
NAME=${2}
DESC="${3}"
LOCN=${4}
HELP=${5}
# dummy out 6th and 7th var.s for ...menu flags
case "$FLAG" in
	"addmenu" | "chgmenu" ) 
				ACTN=""
				TASKS=""
				;;
	"addtask" | "chgtask" )
				ACTN=${6}
				TASKS=${7}
				;;
	"*"  )	exit 9
		;;
esac

# get location name with menu/task name included e.g. main:files:check
# pathname will be $OAMBASE/menu/filemgmt
# part1 = $OAMBASE
# part2 = /menu/filemgmt
# oambase will evaluate to /usr/oam
# oampath = /usr/oam/menu/filemgmt
# newpath = /usr/oam/add-ons/$PKGINST/filemgmt

pathname=`$OAMBASE/edbin/findmenu -o "${LOCN}:${NAME}"`
part1=`$ECHO "${pathname}" | $SED "s/^\([^\/]*\)\/.*/\1/p"`
part2=`$ECHO "${pathname}" | $SED  "s/^[^\/]*\([\/.]*\)/\1/p"`
oambase=`eval $ECHO ${part1}`
oampath=${oambase}${part2}

#now change /menu to /add-ons/PKGINST for correct location
newpath=`$ECHO "${part2}" | $SED  "s/^\/menu/\/add-ons\/$PKGINST/p"`
newpath=${oambase}${newpath}

# need bin path for executables
# binpath = /usr/oam/add-ons/$PKGINST/bin
binpath=${oambase}/add-ons/$PKGINST/bin

# Collision Detection - uses findmenu if FLAG is addmenu or addtask
# Check for collision with existing menu/task w.r.t. adding menu/task
# Error Code = 1

if [ "${FLAG}" = "addmenu" ] || [ "${FLAG}" = "addtask" ]
then

#	Change to direct name.menu path when subdirectories structure used.
#	if $GREP "^${NAME}\^" $oampath/${NAME}.menu

	if $GREP "^${NAME}\^" $oampath/*.menu
	then
	{
		EXITCODE=1
		cleanup
	}
	fi
	if [ "${FLAG}" = "addmenu" ]
	then
		if [ ! -d $newpath ] && [ -n "$newpath" ]
		then
			$MKDIR $newpath
			$MKDIR $newpath\/${NAME}
			cp $HELP $newpath\/${NAME}/Help
		fi
	fi
fi

# Menu Information File Generation - uses mkmf
# Error Code = 2
MI_FILE="`/bin/date +%\H%\M%\S'%j%y'`.mi"

$OAMBASE/edbin/mkmf $ONLINE $MI_FILE $NAME "$DESC" $LOCN $HELP $ACTN 2>/dev/null || {
		EXITCODE=2
		cleanup
		}

# create temporary files for log files and menu files (2nd assignment)
log_file="log.${NAME}"
log_2="log2.${NAME}"
expr_log="expr.${NAME}"

# Copy Task Files  - ONLY for add/chg task
# Error Code = 3

if [ "${FLAG}" = "addtask" ] || [ "${FLAG}" = "chgtask" ]
then
	newdir="${newpath}/${NAME}"

# Make directories if non-existent & 
# Check to see if directories created okay
	if [ ! -d $binpath ]
	then
		$MKDIR -m 755 -p $binpath 
		if [ ! -d $binpath ]
		then
			EXITCODE=3
			cleanup
		else
			chown bin ${binpath}
			chgrp bin ${binpath}
			chown bin ${binpath}
			chgrp bin ${binpath}
		fi
	fi

	if [ ! -d $newdir ]
	then
		$MKDIR -m 755 -p $newdir 
		if [ ! -d $newdir ]
		then
			EXITCODE=3
			cleanup
		else
			chown bin ${newpath}
			chgrp bin ${newpath}
			chown bin ${newdir}
			chgrp bin ${newdir}
		fi
	fi

#  	Copy files

	$ECHO "ACTN=${ACTN}" > /tmp/cptasks
	$ECHO "HELP=${HELP}" >> /tmp/cptasks
	$ECHO "TASK=${TASKS}" | $SED 's/\,/\
TASK=/gp' >>/tmp/cptasks
	copy_tasks
	if [ $EXITCODE -ne 0 ]
	then
		cleanup
	fi

fi

# Modify Menus - uses mod_menus 
# Error Code = 4

/usr/sadm/install/bin/mod_menus -o $MI_FILE $log_file $expr_log 2>>/dev/null || {
	$CAT $log_file | $GREP -v "NEWDIR" | $SORT -d -u | 
	     $CUT -f1 -d" " >/tmp/rmmodtmp
	EXITCODE=4
	cleanup
	}


# Commit Changes
# Error Code = 8
EXITCODE=8

# Sort log file entries and remove duplicates
$SORT -d -u -o $log_file $log_file 2>>/dev/null || {
	$CAT $log_file | $GREP -v "NEWDIR" | 
	     $CUT -f1 -d" " >/tmp/rmmodtmp
	cleanup
	}

# Remove "NEWDIR" entries from log file
$GREP -v "NEWDIR" $log_file > $log_2 2>/dev/null || {
	$CAT $log_2 | $CUT -f1 -d" " >/tmp/rmmodtmp
	cleanup
	}

# Move temp menu file to permanent menu file in log file
$SED 's/^\(.*\)$/mv \1/' $log_2 > $log_file 2>/dev/null || {
	$CAT $log_2 | $CUT -f1 -d" " >/tmp/rmmodtmp
	cleanup
	}

# Execute log file
. ./$log_file 2>/dev/null || {
	$CAT $log_2 | $CUT -f1 -d" " >/tmp/rmmodtmp
	cleanup
	}

# Modify Express Mode Lookup File - uses ie_build
# Error Code = 5

# Sort express log file created from mod_menus - use special sort 
$SORT -t\^ +0d -1d +3r -o $expr_log $expr_log 2>>/dev/null || {
	$CAT $log_file | $GREP -v "NEWDIR" | $SORT -d -u | 
	     $CUT -f1 -d" " >/tmp/rmmodtmp
	EXITCODE=5
	cleanup
	}
# Call ie_build with sorted express log file
/usr/sadm/install/bin/ie_build 2>/dev/null || {
	$CAT $log_file | $GREP -v "NEWDIR" | $SORT -d -u | 
	     $CUT -f1 -d" " >/tmp/rmmodtmp
	EXITCODE=5
	cleanup
	}
# Changes completed without error

# Successful Completion
EXITCODE=0
cleanup
