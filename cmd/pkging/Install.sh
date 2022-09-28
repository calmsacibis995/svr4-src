#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#	Copyright (c) 1987, 1988 Microsoft Corporation
#	  All Rights Reserved

#	This Module contains Proprietary Information of Microsoft
#	Corporation and should be treated as Confidential.

#ident	"@(#)pkging:Install.sh	1.3"
#
# ----------------------------------------------------------------------
# PURPOSE:  Install application software in PLUS format.
# ----------------------------------------------------------------------
INDIR=/usr/lib/installed
CONTENTS=${INDIR}/CONTENTS
PLUSDIR=/tmp/installed
TMPDIR=/usr/tmp/install$$
CPLOG=/tmp/cplog$$
PATH=/sbin:/usr/sbin:/etc:/usr/bin
export PATH

trap "trap '' 1 2 3 9 15; cd /; rm -rf ${TMPDIR} ${CPLOG} > /dev/null 2>&1; echo You have canceled the installation.; sync; exit 1" 1 2 3 9 15

SPACE=/etc/conf/bin/idspace

FDMESS="Please insert the floppy disk.\n\nIf the program installation \
requires more than one floppy disk, be sure to insert the disks in the \
proper order, starting with disk number 1.\nAfter the first floppy disk, \
instructions will be provided for inserting the remaining floppy disks."
ERROR2="An error was encountered while reading in the floppy disk(s).\
Please be sure to insert them in the proper order, that the drive door is closed,\
and wait for the notification before removing them."
ERROR3="If this problem reoccurs at the same floppy disk, the floppy disk may \
be bad. Please re-insert the first floppy disk of this package and try again."

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

if [ $# -eq 2 ]
then DRIVE=$1
else DRIVE=/dev/rdsk/f0
fi
FD=$2
cd /
rm -rf ${TMPDIR}
mkdir ${TMPDIR}
cd ${TMPDIR}

if [ ! -f ${CONTENTS} ]
then	rm -rf ${CONTENTS}
	>> ${CONTENTS}
fi

DEVICE=q
while [ 1 ]
do
	rm -rf Size
	echo "$CLEAR"
	echo "\n\nSearching for the Size file\n"

	xtract icBu Size ${DRIVE} 1>${CPLOG} 2>&1
	err=$?
	sync
	if [ "`grep \"error - Size not found\" ${CPLOG}`" ]
	then DEVICE=q
	elif [ "$err" != "0" ]
	then continue
	fi
	if [ -r Size -a -s Size ]
	then 
		DEVICE=w
		if grep "USR=" Size 2>&1 > /dev/null ||
		   grep "ROOT=" Size 2>&1 > /dev/null
		then
			APPTYPE="386"
		else
			APPTYPE="PLUS"
			# DENSITY is used to flag whether the application
			# should by cpio'd or cpiopc'd.  All plus applications
			# should be cpiopc'd regardless if they are 1.2 or 360.
			DENSITY=360
		fi
	fi
	while [ 1 ]
	do
	if [ $DEVICE = q ]
	then
	    while [ 1 ]
	    do
		flp=0
		while [ $flp -lt 1 ]
		do
			echo "Please enter the number of floppies in the package followed by ENTER: \c"
			read num
			if [ -z "$num" ]
			then
				echo "The number of floppies field must be filled in."
			else
				expr ${num} \* 20 > /dev/null 2>/dev/null
				if  [ $? != 0 ]
					then echo "The number of floppies, is not a valid number."
				elif  [ 1 -gt "$num" ]
				then
					echo "The number of floppies must be at least 1."
				else
					flp=`expr $num`
				fi
			fi
		done
		media=0
		while [ $media -lt 1 ]
		do
			echo "\rPlease enter:\n\
			1 (for 360 KB)\n\
			2 (for 1.2 MB)\n\
			3 (for 720 KB)\n\
			4 (for 1.44 MB)\n\c"
				
			read num
			if [ -z "$num" ]
			then
				echo "The disk density must be filled in."
			else
				expr ${num} \* 20 > /dev/null 2>/dev/null
				if  [ $? != 0 ]
					then echo "The density number is not a valid number."
				elif  [ 1 -eq "$num" ]
				then
					media=`expr $num`
					DENSITY=360
					DEVICE=y
					BLOCKS=702
				elif  [ 2 -eq "$num" ]
				then
					media=`expr $num`
					DEVICE=z
					BLOCKS=2370
				elif [ 3 -eq "$num" ]
				then
					media=`expr $num`
					DEVICE=a
					BLOCKS=1440
				elif [ 4 -eq "$num" ]
				then
					media=`expr $num`
					DEVICE=b
					BLOCKS=2880
				else
					echo "The density number must be 1, 2, 3, or 4."
				fi
			fi
		done
		break
	   done
	   USR=`expr ${flp} \* ${BLOCKS} `
	   ROOT=50
	fi
	if [ $DEVICE = w ]
	then
		linecnt=`expr \`cat Size | wc -l\``
		if [ \( $linecnt -ne 1 \) -a \( $linecnt -ge 3 \) ]
		then
			echo "Invalid Size file found.  Cannot determine disk requirements."
			DEVICE=q
			continue
		fi
		ROOT=10
		USR=10
		if [ "`grep ROOT= Size`" ]
		then ROOT=50
		     USR=50
		fi
		if [ "`grep USR= Size`" ]
		then USR=50
		fi
		if [ \( \( $ROOT = 50 \) -a \( $USR != 50 \) \) -o \( \( $ROOT != 50 \) -a \( $USR = 50 \) \) ]
		then
			echo "Invalid Size file found.  Cannot determine disk requirements."
			DEVICE=q
			continue
		fi
		if [ "`grep USR= Size`" ]
		then
			ROOT=`cat Size | grep ROOT | cut -d= -f2`
			USR=`cat Size | grep USR | cut -d= -f2`
		elif [ "`grep ROOT= Size`" ]
		then
			ROOT=`cat Size | grep ROOT | cut -d= -f2`
		else
			USR=`cat Size`
			ROOT=50
		fi
	fi
	break
	done
	FILE_S="user (/usr) filesystem"
	$SPACE -u $USR > /dev/null 2>&1
	ERR=$?
	if [ $ERR = 3 ]
	then
		FILE_S="hard disk"
		ROOT=`expr $ROOT + $USR`
	fi
	if [ $ERR = 2 ]
	then USR="user (/usr) "
	else USR=
	fi
	$SPACE -r $ROOT > /dev/null 2>&1
	if [ $? = 2 ]
	then ROOT="root (/) "
	else ROOT=""
	fi
	if [ \( $ERR = 2 \) -o \( "$ROOT" = "root (/) " \) ]
	then
		if [ \( $ERR = 2 \) -a \( "$ROOT" = "root (/) " \) ]
		then S=s
		     CONJ=" and "
		else S=
		     CONJ=
		fi
		if [ -z "$USR" ]
		then PART="${ROOT}filesystem "
		else PART="${ROOT}${CONJ}${USR}filesystem${S} "
		fi
		message "There is not enough room on the hard disk to \
install the package.  Please remove some files from the ${PART}and try again."
		echo "The Installation is canceled."
		break
	fi

	if [ $DEVICE != w ]
	then
		message -c "$FDMESS"
		if [ $? != "0" ]
		then
			echo "The Installation is canceled."
			break
		fi
	fi
	rm -rf *
##
	if [ "${APPTYPE}" = "PLUS" ]
	then
		cd /
		rm -rf ${TMPDIR}
		TMPDIR=${PLUSDIR}
		mkdir ${TMPDIR}
		cd ${TMPDIR}
	fi
	echo "$CLEAR"
	echo "                        Install in progress\n\n\n\n\n\n"
	if [ "$DENSITY" = "360" ] && expr "$FD" : '^[0-5]$' > /dev/null
	then
		/usr/sbin/.cpiopc -iBcduw$FD 2>${CPLOG}
		err=$?
	else
		cpio -iBcdu -I${DRIVE} -M"You may remove this floppy disk.
To QUIT - strike <q> followed by <ENTER>
To continue - insert floppy disk number %d and strike the <ENTER> key." 2>${CPLOG}
		err=$?
	fi
	if [ "`grep \"file header information\" ${CPLOG}`" ]
	then
		message -c "The floppy disk you inserted is either not the correct floppy disk, \
or you inserted it in the wrong order.  ${ERROR3}"
		if [ $? != "0" ]
		then
			echo "The Installation is canceled."
			break
		fi
	elif [ "`grep \"annot create\" ${CPLOG}`" ]
	then
		message "Your ${FILE_S} is out of space.  Please remove some files and try again."
		echo "Installation aborted."
		break
	elif [ "$err" = "4" ]
	then
		message -c "You have canceled the Installation.  If you wish \
to try it again press ENTER, otherwise press ESC."
		if [ $? != "0" ]
		then
			echo "The Installation is canceled."
			break
		fi
	elif [ "$err" != "0" ] 
	then
		message -c "${ERROR2}  ${ERROR3}"
		err1=$?
		if [ $err1 != "0" ]
		then
			echo "The Installation is canceled."
			break
		fi
# No need to check for ./Size since we're here
	elif [ ! -f ./Install -o ! -f ./Remove -o ! -f ./Name ]
	then
		message -c "The software package is missing the necessary installation programs.  \
Please check to make sure you have the right floppy disk(s)."
		if [ "$?" != "0" ]
		then	
			echo "The Installation is canceled."
			break
		fi
	else
		trap '' 1 2 3 9 15
		chmod +x ./Install
		NAME=`cat -s Name`

# Check for special case of Simultask on top of MERGE 386 or vice versa
		STASK="Simul-Task 386"
		MG386="Merge 386"
		grep "$STASK" Name > /dev/null 2>&1; stask_new=$?
		grep "$MG386" Name > /dev/null 2>&1; mg386_new=$?
		grep "$STASK" /usr/options/* > /dev/null 2>&1; stask_in=$?
		grep "$MG386" /usr/options/* > /dev/null 2>&1; mg386_in=$?
		if [ "$stask_new" = 0 -a "$mg386_in" = 0 ]
		then
			message -d "You cannot install $STASK on a system \
that has $MG386 installed. You must remove $MG386 before continuing."
			break
		fi
		if [ "$stask_in" = 0 -a "$mg386_new" = 0 ]
		then
			message -d "You cannot install $MG386 on a system \
that has $STASK installed. You must remove $STASK before continuing."
			break
		fi

# Check out if previously installed
		SNAME=`echo "$NAME" | sed -e 's/[ &()$#\-?\\!*;|<>]/./g' -e 's/\[/./g' -e 's/\]/./g'`

		#  Warn the user if there is a xenix package already installed
		#  with the same name.  
		#  Use a bogus directory to check if /etc/perms is an empty 
		#  directory.  Used to be guaranteed not empty in XENIX.
		#  Avoids expansion of '*'.
		tmp=/tmp/CUSLIST
		perm_files=`cd /etc/perms; echo *`
		if [ "/att/msoft/isc/*" != "/att/msoft/isc/$perm_files" ]
		then
			for perm in $perm_files
			do
				#  Call fixperm to list the installed packages 
				#  for this set
				fixperm -iv $ignorepkgs /etc/perms/$perm |
				sed "s/^\(.*\)	.*$/s:^#!\1[ 	][ 	]*:	&	:p/" > $tmp.fl
				#  Get the package name in field 4 
				sed -n -f $tmp.fl /etc/perms/$perm   | 
						sed "s/^	//"  | 
						cut -f4 >> $tmp.sid
			done

			KEY=`grep "$SNAME\$" $tmp.sid`
			rm -f $tmp.sid $tmp.fl
			if [ -n "$KEY" ]
			then 
				message -c "A XENIX package with the same name, $NAME, \
	has already been installed.  This package may overwrite the XENIX package."
			     if [ "$?" != "0" ]
			     then echo "The Installation is canceled"
				 break
			     fi
			fi
		fi

		KEY=`grep " $SNAME\$" $CONTENTS`
		if [ -n "$KEY" ]
		then message -c "The $NAME has already been installed.  \
The new installation will now replace the original $NAME files."
		     if [ "$?" != "0" ]
		     then echo "The Installation is canceled"
			 break
		     fi
		     cd /usr/options
		     for i in *
		     do
			grep "^${SNAME}$" $i > /dev/null 2>&1
			if [ $? = 0 ]
			then
				KEY2=$i
				break
			fi
		     done
		     cd ${TMPDIR}
		     eval KEY=`grep " $SNAME\$" $CONTENTS | cut -f1 -d' ' `
		     if [ "$KEY" ]
		     then
			   mv ${INDIR}/Remove/"${KEY2}" /tmp/"${KEY}".R
			   mv ${INDIR}/Files/"${KEY2}" /tmp/"${KEY}".F > /dev/null 2>&1
			   mv /usr/options/"${KEY2}" /tmp/"${KEY}".O
			   grep " $SNAME\$" $CONTENTS > /tmp/"${KEY}"
		           grep -v " $SNAME\$" $CONTENTS > /tmp/$$
		           mv /tmp/$$ $CONTENTS
		     fi
		else KEY2=`echo "$NAME" | sed -e 's/ //g' -e 's/\///g' -e 's/\&//g' | tr "[A-Z]" "[a-z]" | cut -c1-6`
		     if [ -f /usr/options/"${KEY2}".name ]
		     then
			num=`expr 1`
			while [ -f /usr/options/"${KEY2}"${num}.name ]
			do
				num=`expr $num + 1`
			done
			KEY2="${KEY2}"${num}.name
		     else
			KEY2="${KEY2}".name
		     fi
		fi
		echo "$CLEAR "
		./Install
		# It is the 'Install' script's job to print an error message
		# if the installation indicates the return of non-zero:
		if [ "$?" != "0" ]
		then
			if [ -n "$KEY" ]
			then
				mv /tmp/"${KEY}".R ${INDIR}/Remove/"${KEY2}" > /dev/null 2>&1
				mv /tmp/"${KEY}".F ${INDIR}/Files/"${KEY2}" > /dev/null 2>&1
				mv /tmp/"${KEY}".O /usr/options/"${KEY2}" > /dev/null 2>&1
				cat /tmp/"${KEY}" >> $CONTENTS
				rm -f /tmp/"${KEY}" > /dev/null 2>&1
			fi
			break
		else
			rm -f /tmp/"${KEY}".[RFO] /tmp/"${KEY}" > /dev/null 2>&1
			if [ ! -d $INDIR ]
			then mkdir $INDIR; chmod 755 $INDIR
			fi
			if [ ! -d ${INDIR}/Files ]
			then mkdir ${INDIR}/Files; chmod 755 ${INDIR}/Files
			fi
			if [ ! -d ${INDIR}/Remove ]
			then mkdir ${INDIR}/Remove; chmod 755 ${INDIR}/Remove
			fi
			echo "$KEY2 1 $NAME" >> $CONTENTS
			mv ./Remove ${INDIR}/Remove/"${KEY2}"
			mv ./Name /usr/options/"${KEY2}"
			if [ -f Files ]
			then
				mv Files ${INDIR}/Files/"${KEY2}"
			fi
			chown bin ${INDIR}/Remove/"${KEY2}" /usr/options/"${KEY2}" ${INDIR}/Files/"${KEY2}" > /dev/null 2>&1
			chgrp bin ${INDIR}/Remove/"${KEY2}" /usr/options/"${KEY2}" ${INDIR}/Files/"${KEY2}" > /dev/null 2>&1
			chmod 644 ${INDIR}/Remove/"${KEY2}" /usr/options/"${KEY2}" ${INDIR}/Files/"${KEY2}" > /dev/null 2>&1
			sync; sync
			if [ -f /etc/.new_unix ]
			then
				sync; sync
				cd /
				rm -rf ${TMPDIR} ${CPLOG}
				exec /etc/conf/bin/idreboot
			else
				message -d "The installation of the \
$NAME is now complete."
			fi
			break
		fi
	fi
	DEVICE=q
done
cd /
rm -rf ${TMPDIR} ${CPLOG}
