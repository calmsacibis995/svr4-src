#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:edsysadm/mkmf.sh	1.5.2.1"

################################################################################
#	Module Name: mkmf
#
#	Input: 
#		$1 -> flag to indicate menu/task and change/add
#			- chgmenu
#			- chgtask
#			- addmenu
#			- addtask
#			- online
#		$2 -> name of package description file 
#		      tmp file name for online flag
#		$3 -> name (changed name)
#		$4 -> description 
#		$5 -> location (changed location)
#		$6 -> help file
#		$7 -> action (Forms only)
#		$8 -> location:name (original "location:name")
#
#	Output:
#          	task -> location:name^Description of Menu Item^Help file^Action
#          	menu -> location:name^Description of Menu Item^Help file^
#
#	Processing:
#		1) Check flag and determine output destination.  Possible
#		   destinations are:
#			- *.mi file from package info file
#			- create new *.mi file
#			- online output goes to a temp file
#			- *.mi file in current directory
#		2) For change menu/task, delete original entry.
#
#		3) Create entry for menu information file
#
#		4) Check for duplicate entries.
#
#		5) Add menu information file to prototype file.
#
# 	UNIX commands:
#		grep, cut, mv, ls, wc, date, echo, pwd
#
################################################################################

# Exit values
SUCCESS=0
INVALID=1
DUP_ENTRY=2
TOO_MANY=4

# Path name where menu information file goes
MIPATH=/var/sadm/pkg/intf_install/\$PKGINST

# Temporary menu information buffer
TMP_MI=/tmp/$$mi

# Menu/task logical location
LOC="$5:$3"

# Assign arguments to descriptive variables
FLAG="$1"
MI_FILE="$2"
NAME="$3"
DESCRIP="$4"
LOCATION="$5"
HELP="$6"
ACTION="$7"
ORIGLOC="$8"

# Tools bin
EDBIN=/$OAMBASE/edbin

##########################################################################
# The following code checks flag options and determines the destination
# of the menu information entries.
##########################################################################

if [ "$FLAG" = "chgmenu" ] || [ "$FLAG" = "chgtask" ]
then
	# delete original entry from menu information file
	grep -v "^$ORIGLOC^" $MI_FILE > /tmp/$$mifile
	mv /tmp/$$mifile  $MI_FILE
		
elif [ "$FLAG" = "addmenu" ] || [ "$FLAG" = "addtask" ]
then
	# must be addmenu or addtask
	# find number of menu information file exist in current directory
	filecnt=`ls *.mi 2> /dev/null | wc -w`

	# Does a menu information file exist??
	if [ $filecnt -eq 0 ]
	then
		#   Set variable to hour, minute, second, day-of-year
		#   and year to guarantee uniqueness of the .mi file

		MI_FILE="`date +%\H%\M%\S'%j%y'`.mi"

	# Check for multiple menu information files
	elif [ $filecnt -gt 1 ]
	then
		exit $TOO_MANY
	
	# Only 1 menu information file
	else
		MI_FILE=`ls *.mi`

	fi
fi

########################   END FLAG OPTION CHECK    #################################

# Check for duplicate entries

if [ -r *.mi ]
then
	grep "^$LOC^" $MI_FILE > /dev/null && exit  $DUP_ENTRY
fi

####################################################################
#   Append entry to menu infomation file
#   EXAMPLE ENTRY:
#          location:name^Description of Menu Item^Help file^Action
#   Only task items have action files, for menu items the action
#
#   file does not exist
####################################################################
#

# REMOVED THE HELP FILE FOR NOW - 8/8/88
#echo ""$LOCATION:$NAME"^"$DESCRIP"^"$HELP"^"$ACTION"" >> $MI_FILE
echo ""$LOCATION:$NAME"^"$DESCRIP"^"$ACTION"" >> $MI_FILE


####################################################################################
# Add menu infomation file (*.mi) to prototype file
# EXAMPLE:
#	 1              2    3               4               5           6
#   mkpf prototype file flag "pkgdesc file" "location:name" "help file" "comma separated file list"
####################################################################################

if [ -w "$MI_FILE" ]
then
	grep $MIPATH prototype > /dev/null
	if [ $? -ne 0 ]
	then

	  $EDBIN/mkpf prototype "mifile" "" "$MIPATH" "" "`pwd`/$MI_FILE" || exit
	fi
fi

exit $SUCCESS
