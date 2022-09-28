#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:edsysadm/mkpkg.sh	1.4.2.1"

################################################################################
#	Module Name: mkpkg
#
#	Inputs: 
#		$1 -> flag to indicate menu/task and change/add
#			- chgmenu
#			- chgtask
#			- addmenu
#			- addtask
#			- overwrite
#		$2 -> name of package description file
#		$3 -> menu/task name
#		$4 -> description field of menu/task
#		$5 -> location of menu/task
#		$6 -> help file
#		$7 -> task action (Task only)
#		$8 -> comma separated list of task files (Task only)
#		$9 -> original location (LOCATION:NAME)
#
# 	UNIX commands:
#		cut, grep, echo, pwd, ls
#
################################################################################

# Assing arguements to descriptive variables
FLAG="$1"
PKGDESC="$2"
NAME="$3"
DESCRIP="$4"
LOCATION="$5"
HELP="$6"
ACTION="$7"
TASKFILES="$8"
ORIGLOC="$9"

# Tools bin
EDBIN=$OAMBASE/edbin

# Check status of package description file
# If changes are to be made and the package description file
# does not exist or is not writeable - exit process with NOTFND

if [ "$FLAG" = "chgmenu" -o "$FLAG" = "chgtask" -o "$FLAG" = "overwrite" ]
then
	# Check if package description file exist and is writable.
	if [ ! -w "$PKGDESC" ]
	then
		# Package description file not found
		exit 1
	# Extract menu information file and prototype files
	# from the package description file.
	else
		ITEM=`grep "#$ORIGLOC^" "$PKGDESC"`
		MIFILE=`echo $ITEM | cut -d\^ -f 5`
		PROTO=`echo $ITEM | cut -d\^ -f 6`
	fi

	# Check if prototype file exist and is writable.
	if [ ! -w $PROTO ]
	then
		exit $INVALID
	fi
# Must be an add - set prototype file to current directory
else
	PROTO=prototype
fi


# Check action flag and add/update the following files:
#		-> prototype file
#		-> menu information file
#		-> package description file

# MENU
if [ $1 = chgmenu ] || [ $1 = addmenu ]
then
	#   Make a prototype entry for help directory and help file
	#      --> mkpf prototype flag pkgdesc loc:name help
	
	$EDBIN/mkpf "$PROTO" "$FLAG" "$PKGDESC" "$LOCATION:$NAME" "$HELP" || exit

	#   Call 'mkmf' to create and/or update menu 
	#   information (*.mi) file.  Called from Form.menu,
	#   there is no action passed in.

	$EDBIN/mkmf "$FLAG" "$MIFILE" "$NAME" "$DESCRIP" "$LOCATION" "" "" "$ORIGLOC" || exit

	# If flag is an add then set MIFILE and PROTO
	# to file in current directory.
	if [ "$FLAG" = "addmenu" ]
	then
		MIFILE="`pwd`/`ls *.mi`"
		PROTO="`pwd`/prototype"
	fi
		

# TASK
else
	#   Create and/or update prototype file
	#      --> mkpf prototype flag pkgdesc loc:name help action,files
	
	# If action file should be included just add $ACTION, to TASKFILES
	$EDBIN/mkpf "$PROTO" "$FLAG" "$PKGDESC" "$LOCATION:$NAME" "$HELP" \
		     "$TASKFILES,$ACTION" || exit

	#   Call 'mkmf' to create and/or update menu 
	#   information (*.mi) file.
	#   Must have been called from Form.task,
  	#   and an action was passed in.

	$EDBIN/mkmf "$FLAG" "$MIFILE" "$NAME" "$DESCRIP" "$LOCATION" \
		     "$HELP" "$ACTION" "$ORIGLOC" || exit

	# If flag is an add then set MIFILE and PROTO
	# to file in current directory.
	if [ $1 = addtask ]
	then
		MIFILE="`pwd`/`ls *.mi`"
		PROTO="`pwd`/prototype"
	fi

fi

$EDBIN/updt_pkgdesc "$FLAG" "$PKGDESC" "$LOCATION:$NAME" "$DESCRIP" \
 		 "$HELP" "$ACTION" "$MIFILE" "$PROTO" "$TASKFILES" "$ORIGLOC" || exit

exit 0
