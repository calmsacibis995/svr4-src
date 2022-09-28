#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:edsysadm/test_menu.sh	1.4.2.1"
#################################################################################
#	Module Name: test_menu
#
#	Inputs:
#		$1 -> flag to indicate menu change/add
#			- chgmenu
#			- addmenu
#		$2 -> menu name
#		$3 -> description field of menu
#		$4 -> location of menu
#		$5 -> help file
#
#	Description:
#		This process allows for online testing of menus that
#		are added or modified through edsysadm.  It will show the
#		user how the menu will appear in the logical directory
#		structure.
#
#	UNIX commands:
#		basename, cat, cp, cpio, cut, echo, expr, find, grep
#		mkdir, rm, sed, sort
################################################################################

trap cleanup 1 2 3 15

# Assign temporary files for log files and menu file
MI_FILE=/tmp/$$.mi
LOG_FILE=/tmp/$$.log
LOG_2=/tmp/$$.log2
EXPR_LOG=/tmp/$$.expr
EXPR_2=/tmp/$$.exp2
RMMODTMP=/tmp/$$rmmodtmp
# Need to try ${VPID}
ITEMNAME=/tmp/${VPID}.arg1
MENUPATH=/tmp/${VPID}.arg2
MENUNAME=/tmp/${VPID}.arg3

# Assign arguments to descriptive variables
FLAG=$1
NAME=$2
DESC="$3"
LOCATION=$4
# Check if help file is absolute or relative path
if expr "$5" : '^[^/]' > /dev/null
then
	HELP=`pwd`/$5
else
	HELP=$5
fi

# Exit Codes
SUCCESS=0
COLLISION=1
MKMF_ERR=2
DUP_STRUCT=3
MOD_MENU=4
EXITCODE=$SUCCESS

# Tools Bin
EDBIN=$OAMBASE/edbin
INSTBIN=/usr/sadm/install/bin

#################################################################################
#
#	Module Name: cleanup
#
#	Description:
#		Remove temporary file, reset OAMBASE and exit.
#################################################################################
cleanup() {

# Remove the temp files created by mod_menus if created and error exits.
if [ -s $RMMODTMP ]
then
	for i in `cat $RMMODTMP`
	do
		rm -f $i
	done
fi

rm -f $MI_FILE 2>/dev/null \
      $LOG_FILE 2>/dev/null \
      $LOG_2 2>/dev/null \
      $EXPR_LOG 2>/dev/null \
      $EXPR_2 2>/dev/null


#rm -f $MI_FILE 2>/dev/null
#rm -f $LOG_FILE 2>/dev/null
#rm -f $LOG_2 2>/dev/null
#rm -f $EXPR_LOG 2>/dev/null
#rm -f $EXPR_2 2>/dev/null

exit $EXITCODE
}

#################################################################################
#
#	Module Name: coll_detect
#
#	Description: Check for collision between new additions and
#		     the existing Interface menu definition
#
#	example: main:applications:ndevices
#	
#		pathname --> $OAMBASE/menu/applmgmt
#		part1    --> $OAMBASE
#		part2    --> menu/applmgmt
#		oambase  --> /usr/sadm/sysadm (or whatever $OAMBASE is)
#		PATHNAME --> /usr/sadm/sysadm/menu/applmgmt
#################################################################################
coll_detect() {

# get the physical location of menu
pathname=`$EDBIN/findmenu -o "$LOCATION:$NAME"`            
part1=`echo "$pathname" | sed "s/^\([^\/]*\)\/.*/\1/p"`   
part2=`echo "$pathname" | sed  "s/^[^\/]*\([\/.]*\)/\1/p"`
oambase=`eval echo $part1`
testbase=`eval echo $TESTBASE`
PATHNAME=$oambase$part2
TMP_PATH=$testbase$part2

#  Change to direct name.menu path when subdirectories structure used.
#  if grep "^${NAME}\^" $PATHNAME/${NAME}.menu

if grep "^${NAME}\^" `eval echo $PATHNAME`/*.menu
then
	EXITCODE=$COLLISION
	cleanup
fi
}

#################################################################################
#
#	Module Name: dup_struct
#
#	Description:
#		Duplicate menu structure in $TESTBASE
#################################################################################
dup_struct(){

mkdir -m755 -p $TMP_PATH
 
cd $PATHNAME

cp $OAMBASE/menu/main.menu $TESTBASE/menu
find . -print | cpio -pdum $TMP_PATH
	
	
}

###########################################################
# main function
#	Set pkginst to _ONLINE
#	Collision Detection
#	Menu Info File Generation
#	Duplicate affected Menu Structure
#	Reset OAMBASE
#	Modify Menus
#	Commit Changes
#	Copy help file
#	Generate parent menu
###########################################################

# Set Pkginst Variable to _ONLINE
PKGINST=_ONLINE
export PKGINST

# Collision Detection
#if [ "$FLAG" = "addmenu" ]
#then
	coll_detect
#fi

# Menu Information File Generation - uses mkmf

$EDBIN/mkmf "online" "$MI_FILE" "$NAME" "$DESC" "$LOCATION" "$HELP" 2>/dev/null || {
		EXITCODE=$MKMF_ERR
		cleanup
		}

# Duplicate the menu structure in $TESTBASE
dup_struct

# Modify Menus - uses mod_menus 

$INSTBIN/mod_menus -t $MI_FILE $LOG_FILE $EXPR_LOG 2>>/dev/null || {
	cat $LOG_FILE | grep -v "NEWDIR" | sort -d -u | 
	     cut -f1 -d" " >$RMMODTMP
	EXITCODE=$MOD_MENU
	cleanup
	}

# Commit Changes
EXITCODE=8

# Sort log file entries and remove duplicates
sort -d -u -o $LOG_FILE $LOG_FILE 2>>/dev/null || {
	cat $LOG_FILE | grep -v "NEWDIR" | 
	     cut -f1 -d" " >$RMMODTMP
	cleanup
	}

# Remove "NEWDIR" entries from log file
grep -v "NEWDIR" $LOG_FILE > $LOG_2 2>/dev/null || {
	cat $LOG_2 | cut -f1 -d" " >$RMMODTMP
	cleanup
	}

# Move temp menu file to permanent menu file in log file
sed 's/^\(.*\)$/mv \1/' $LOG_2 > $LOG_FILE 2>/dev/null || {
	cat $LOG_2 | cut -f1 -d" " >$RMMODTMP
	cleanup
	}

# Execute log file
#. $LOG_FILE 2>/dev/null || {
. $LOG_FILE || {
	cat $LOG_2 | cut -f1 -d" " >$RMMODTMP
	cleanup
	}

# Changes completed without error
EXITCODE=$SUCCESS

# Copy help file to temporary menu structure
mkdir $TMP_PATH/$NAME/HELP

cp $HELP $TMP_PATH/$NAME/HELP

# May want to try setting global variables??
# Create 3 temporary files that will hold Menu.testmenu arguments
echo $NAME > $ITEMNAME
echo $TMP_PATH > $MENUPATH
#ASSUME THERE IS ONLY 1 '*.menu' file; is this a correct assumption?? 
echo `basename $PATHNAME/*.menu` > $MENUNAME

cleanup
