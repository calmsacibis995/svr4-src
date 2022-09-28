#!/sbin/sh

#ident	"@(#)pkging:Install.tape	1.3"

# ----------------------------------------------------------------------
# PURPOSE: Install application software from tape (V/386 3.2 version 2.2
# ----------------------------------------------------------------------

INDIR=/usr/lib/installed
CONTENTS=${INDIR}/CONTENTS
TMPDIR=/usr/tmp/install$$
CPLOG=/tmp/cplog$$
SPACE=/etc/conf/bin/idspace
TMP_FILE=/tmp/T$$
PACK_FLNM=/tmp/pkglist
MAX_CHECK=8
PATH=/sbin:/usr/sbin:/etc:/usr/bin
export PATH

[ -z "$TERM" ] && TERM=vt100
CLEAR=`tput clear 2>/dev/null`

if [ $# -gt 0 ]
then SPECIAL=YES
fi
export SPECIAL
DEVICE=/dev/rmt/c0s0
DEVICEN=""
SCSI_DEV=/dev/scsi/qtape1
OATAPE=ctape1

TAPECNTL=/usr/lib/tape/tapecntl
if [ -c ${SCSI_DEV} ]
then DEVICE=${SCSI_DEV}
     DEVICEN="-y ${DEVICE}n"
     TAPECNTL=/usr/lib/scsi/scsitapecntl
     OATAPE=qtapeN
fi
if [ -c /dev/rmt/c0s0 -a -c ${SCSI_DEV} ]
then
/etc/machid -s
if [ $? = 1 ]
then DEVICE=${SCSI_DEV}
     DEVICEN="-y ${DEVICE}n"
     TAPECNTL=/usr/lib/scsi/scsitapecntl
     OATAPE=qtapeN
else DEVICE=/dev/rmt/c0s0
     DEVICEN=""
     TAPECNTL=/usr/lib/tape/tapecntl
     OATAPE=ctape1
fi
fi
export DEVICE TAPECNTL DEVICEN OATAPE
[ -f ${TAPECNTL} ] || {
	echo "The Program ${TAPECNTL} is missing"
	exit 1
}
TENSION="${TAPECNTL} -t ${DEVICEN}"
REWIND="${TAPECNTL} -w ${DEVICEN}"
MOVE="${TAPECNTL} -p"

# Function for old 3.2.1 style tapes
old_tape()
{
	loop=$2
	while [ ${loop} -le ${MAX_CHECK} ]
	do
	${REWIND} > /dev/null 2>&1
	sleep 1
	${MOVE} ${loop} ${DEVICEN} > /dev/null 2>&1
	sleep 1
	dd if=$1 of=${PACK_FLNM} bs=5120 count=1 > /dev/null 2>&1
	if [ "$?" != "0" ]
	then return 1
	fi
	grep ${PACK_FLNM} ${PACK_FLNM} > /dev/null 2>&1
	if [ $? = "0" ]
	then break
	fi
	loop=`expr ${loop} + 1`
	done

	if [ ${loop} -gt ${MAX_CHECK} ]
	then return 1
	else echo ${loop} > /tmp/pkg.idx
	     return 0
	fi
}

trap "trap '' 1 2 3 9 15; cd /; rm -rf /tmp/pkg.idx ${TMP_FILE} ${PACK_FLNM} ${TMPDIR} ${CPLOG}; echo You have canceled the installation.; (< ${DEVICE}) > /dev/null 2>&1; sync; exit 1" 1 2 3 9 15

#
# variables
#

PKG_BASE=1
PKG_CNT=0
PKG_LIST=""
ERROR=0
INSTALLED_LIST=""

echo "0" > /tmp/pkg.idx
if [ -z "$SPECIAL" ]
then
while [ 1 ]
do
while [ 1 ]
do
message -c "Please insert the cartridge tape into the tape drive."
if [ "$?" != 0 ]
then echo "\nInstallation has been canceled."
	exit 1
fi
( < ${DEVICE} ) > /dev/null 2>&1
if [ $? = 0 ]
then break
fi
sleep 2
echo "\n"
done
echo "\n"
rm -f ${PACK_FLNM}
sleep 3
dd if=${DEVICE} of=$PACK_FLNM bs=5120 count=1 > /dev/null 2>&1
if [ "$?" = "0" ]
then
grep $PACK_FLNM $PACK_FLNM > /dev/null 2>&1
if [ "$?" = "0" ]
then break
fi
sync;sync
seq=`dd if=${DEVICE} bs=5120 count=1 2>/dev/null`
if [ "20" = `expr "$seq" : "# PaCkAgE DaTaStReAm"` ]
then 
	message -d "This package is an OA&M style package.  \
To install this package type in:\n\n\t\tpkgadd -d ${OATAPE}\n"
	exit 1
fi
old_tape ${DEVICE}n 0
if [ "$?" = "0" ]
then ${REWIND} > /dev/null 2>&1
     break
fi
fi
message -c "This tape is not in the correct format.\nNo installable packages \
could be found.\n\nPlease check that the correct tape has been inserted into the drive."
if [ "$?" != "0" ]
then echo "Installation has been canceled."
	(< ${DEVICE}) > /dev/null 2>&1
	cd /; rm -rf /tmp/pkg.idx ${TMP_FILE} ${PACK_FLNM} ${TMPDIR} ${CPLOG}; sync
	exit 1
fi
done

message -c "It is recommended that you re-tension the tape before attempting \
the installation, to ensure that the tape is read without any errors.\n\n\
If you strike ENTER the tape will be re-tensioned."
if [ "$?" != "1" ]
then
	echo "Re-tensioning the tape media.\n\nThis will take approximately 3 minutes."
	sleep 3
	$TENSION
	if [ $? != 0 ]
	then echo "Re-tensioning failed.  Please try again."
		cd /; rm -rf /tmp/pkg.idx ${TMP_FILE} ${PACK_FLNM} ${TMPDIR} ${CPLOG}; sync
		(< ${DEVICE}) > /dev/null 2>&1
		exit 1
	fi
	sleep 2
fi

rm -f ${PACK_FLNM}
sleep 2
${MOVE} `cat /tmp/pkg.idx` ${DEVICEN}
sleep 1
cpio -iBcu -I${DEVICE}n 2>/dev/null
if [ $? != 0 ]
then echo "Can't read the cartridge tape.  Please try again."
	cd /; rm -rf /tmp/pkg.idx ${TMP_FILE} ${PACK_FLNM} ${TMPDIR} ${CPLOG}; sync
	(< ${DEVICE}) > /dev/null 2>&1
	exit 1
fi
fi

OLDTAPE=0
PKG_BASE=`expr \`cat /tmp/pkg.idx\` + 1`
PKG_CNT=`expr \`cat $PACK_FLNM | wc -l\``
grep "^PKG[0-9]" ${PACK_FLNM} > /dev/null 2>&1
if [ "$?" = "0" ]
then OLDTAPE=1
else OLDTAPE=0
fi
if [ \( $PKG_CNT -lt 2 \) -a \( ${OLDTAPE} = "0" \) ]
then echo "This tape is not in the correct format.  Installation has been canceled."
	(< ${DEVICE}) > /dev/null 2>&1
	cd /; rm -rf /tmp/pkg.idx ${TMP_FILE} ${PACK_FLNM} ${TMPDIR} ${CPLOG}; sync
     exit 1
fi

rm -rf ${TMPDIR} ${TMP_FILE}
mkdir ${TMPDIR}
cd ${TMPDIR}

if [ ${OLDTAPE} = "1" ]
then cat $PACK_FLNM > ${TMP_FILE}
else cat $PACK_FLNM | sed -n "2,\$p" | grep -v "	Hidden$" > ${TMP_FILE}
fi
if [ ! -s ${TMP_FILE} ]
then
	message -d "This tape has no 'installpkg format' packages on it.\n\
If this tape has OA&M style packages on it, please run:\n\
\t\tpkgadd -d ${OATAPE}"
	(< ${DEVICE}) > /dev/null 2>&1
	cd /; rm -rf /tmp/pkg.idx ${TMP_FILE} ${PACK_FLNM} ${TMPDIR} ${CPLOG}; sync
	exit 1
fi

echo $CLEAR
if [ -n "$SPECIAL" ]
then STR="Foundation Set Add-on\npackages that you wish to install."
else STR="packages that you\nwish to install from this tape."
fi
echo "You will now be prompted to select the ${STR}\n\n\
You may select one or more packages from the menu by\n\
entering the number listed alongside the package name.\n"
echo "Enter each package number one at a time, pressing ENTER\n\
after each selection.  The package numbers may be entered\n\
in any order.\n"
echo "To install all the packages, type the number indicated\n\
at the end of the package list.\n\n\
When you have made all the selections required, Strike ESC.\n"
echo "To skip this step or cancel any selections made, type\n\
the number as indicated in the package list.\n\n"
echo "Strike ENTER when ready".
read x

while [ 1 ]
do
while [ 1 ]
do
echo $CLEAR
if [ ${OLDTAPE} = "0" ]
then echo "Tape Name: `sed -n 1p $PACK_FLNM`"
fi
echo "\nPackages available for installation:\n"
count=1
if [ ${OLDTAPE} = 1 ]
then
while [ 1 ]
do
	echo "$count.\c"
	grep "${count}=" ${TMP_FILE} | cut -f2- -d= | sed -e "s/^/	/g"
	count=`expr $count + 1`
	grep "${count}=" ${TMP_FILE} > /dev/null 2>&1
	if [ "$?" != "0" ]
	then break;
	fi
done
else
while [ 1 ]
do
	echo "$count.\c"
	grep "	$count$" ${TMP_FILE} | cut -f1 -d"	" | sed -e "s/^/	/g"
	count=`expr $count + 1`
	grep "	$count$" ${TMP_FILE} > /dev/null 2>&1
	if [ "$?" != "0" ]
	then break;
	fi
done
fi
echo "\n${count}.\tInstall ALL packages shown above"
count=`expr $count + 1`
echo "${count}.\tExit, do not install any packages\c"
message -d "Please enter the next package number(s) to install, followed by \
ENTER.\n\nPress ESC when all selections have been made."

/usr/lbin/get_sel $count 2> /tmp/Ap$$
if [ "$?" != "0" ]
then message -c "You have canceled the package selection menu.\n\n\
Stike ENTER if you wish to re-display the menu and re-select."
if [ "$?" != "0" ]
then echo "Installation has been canceled."
	(< ${DEVICE}) > /dev/null 2>&1
	cd /; rm -rf /tmp/pkg.idx ${TMP_FILE} ${PACK_FLNM} ${TMPDIR} ${CPLOG} /tmp/Ap$$; sync
	   exit 1;
else continue;
fi
fi
if [ ! -s /tmp/Ap$$ ]
then echo "Installation has been canceled."
	(< ${DEVICE}) > /dev/null 2>&1
	cd /; rm -rf /tmp/pkg.idx ${TMP_FILE} ${PACK_FLNM} ${TMPDIR} ${CPLOG} /tmp/Ap$$; sync
	   exit 1;
fi
if [ "$?" = "0" ]
then break;
fi
done

echo "$CLEAR"

sort -u -o /tmp/Ap$$ /tmp/Ap$$
echo "\nYou have made the following selections:\n"
if [ ${OLDTAPE} = "1" ]
then
cat /tmp/Ap$$ | while read k
do
	echo "$k.\c"
	grep "$k=" ${PACK_FLNM} | cut -f2- -d= | sed -e "s/^/	/g"
done
else
cat /tmp/Ap$$ | while read k
do
	echo "$k.\c"
	grep "	$k$" ${PACK_FLNM} | cut -f1 -d"	" | sed -e "s/^/	/g"
done
fi
message -c "\nStrike ENTER to confirm and continue with the installation or \
ESC to re-display the menu and re-select."
if [ "$?" = "0" ]
then
	break
fi
done

if [ ! -f ${CONTENTS} ]
then	rm -rf ${CONTENTS}
	>> ${CONTENTS}
fi

message -t "REMINDER!" "Depending on the packages you are installing, you may be \
required to provide some input to the installation utility to configure the \
software for your system."

trap '' 1 2 3 9 15
echo "$CLEAR"
echo "            Installation in progress -- Do not remove the cartridge tape"
if [ -z "$SPECIAL" ]
then CURR_POS=$PKG_BASE
else CURR_POS=2
fi
export CURR_POS
k=0;
if [ ${OLDTAPE} = "1" ]
then cat ${PACK_FLNM} | cut -f1 -d= | cut -f2- -dG > ${TMP_FILE}
else cat ${PACK_FLNM} | sed -n "2,\$p" | cut -f2 -d'	' > ${TMP_FILE}
fi
cat ${TMP_FILE} | while read pkg_id
do
	k=`expr $k + 1`
	grep "${pkg_id}" /tmp/Ap$$ > /dev/null 2>&1
	if [ "$?" != "0" ]
	then continue
	fi
	POS=`expr $PKG_BASE + $k - $CURR_POS - 1` 
	CURR_POS=`expr $CURR_POS + $POS + 1` 

if [ "$NAME" != "" ]
then
echo "\nSearching for the Next Package to Install -- Do not remove the cartridge tape"
fi

	eval $MOVE $POS ${DEVICEN} >/dev/null 2>&1	
	sleep 3
	cpio -iBcdu -I${DEVICE}n 2>${CPLOG}
	if [ $? != "0" ]
	then
		echo "An error was encountered while reading in the tape."
		ERROR=1
		break
	fi
	if [ ! -f ./Size ]
	then
		echo "The software package is missing the Size file"
		ERROR=1
		break
	fi
	# check the disk requirement
	linecnt=`expr \`cat Size | wc -l\``
	if [ \( $linecnt -ne 1 \) -a \( $linecnt -ge 3 \) ]
	then
		echo "Invalid Size file found.  Cannot determine disk requirements."
		ERROR=1
		break
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
		ERROR=1
		break
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
	$SPACE -u $USR > /dev/null 2>&1
	ERR=$?
	[ $ERR = 3 ] && ROOT=`expr $ROOT + $USR`
	$SPACE -r $ROOT > /dev/null 2>&1
	[ $? = 2 ] && ROOT="root (/)"
	if [ \( $ERR = 2 \) -o \( "$ROOT" = "root (/)" \) ]
	then
		echo "There is not enough room on the hard disk to \
install the package."
		ERROR=1
		break
	fi

	if [ ! -f ./Install -o ! -f ./Remove -o ! -f ./Name ]
	then
		echo "The software package is missing the necessary installation programs."
		ERROR=1
		break
	else
		trap '' 1 2 3 9 15
		chmod +x ./Install
		NAME=`cat -s Name`
		# Check out if previously installed
		SNAME=`echo $NAME | sed -e 's/[ &()$#-?\\!*;|<>]/./g' -e 's/\[/./g' -e 's/\]/./g'`
		KEY=`grep " $SNAME\$" $CONTENTS`
		if [ -n "$KEY" ]
		then
		     message -d "The $NAME package has already been installed.  \
The new installation will now replace the original $NAME files."
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
			   mv ${INDIR}/Remove/${KEY2} /tmp/${KEY}.R
			   mv ${INDIR}/Files/${KEY2} /tmp/${KEY}.F
			   mv /usr/options/${KEY2} /tmp/${KEY}.O
			   grep " $SNAME\$" $CONTENTS > /tmp/${KEY}
		           grep -v " $SNAME\$" $CONTENTS > /tmp/$$
		           mv /tmp/$$ $CONTENTS
		     fi
		else KEY2=`echo $NAME | sed -e 's/ //g' | tr "[A-Z]" "[a-z]" | cut -c1-6`
		     if [ -f /usr/options/${KEY2}.name ]
		     then
			num=`expr 1`
			while [ -f /usr/options/${KEY2}${num}.name ]
			do
				num=`expr $num + 1`
			done
			KEY2=${KEY2}${num}.name
		     else
			KEY2=${KEY2}.name
		     fi
		fi
		./Install < /dev/tty
		# It is the 'Install' script's job to print an error message
		# if the installation indicates the return of non-zero:
		if [ "$?" != "0" ]
		then
			if [ -n "$KEY" ]
			then
				mv /tmp/${KEY}.R ${INDIR}/Remove/${KEY2} > /dev/null 2>&1
				mv /tmp/${KEY}.F ${INDIR}/Files/${KEY2} > /dev/null 2>&1
				mv /tmp/${KEY}.O /usr/options/${KEY2} > /dev/null 2>&1
				cat /tmp/${KEY} >> $CONTENTS
				rm -f /tmp/${KEY} > /dev/null 2>&1
			fi
			message -c "The installation of $NAME failed.\n\n\
Strike ENTER if you wish to continue installation with the other packages you have \
chosen or Strike ESC to skip the rest of the packages.  If you Strike ESC any \
packages previously installed will remain installed." < /dev/tty
			if [ "$?" = "0" ]
			then continue
			fi
			break
		else
			if [ -n "$KEY" -a -s /tmp/${KEY}.un ]
			then
				rm -f /tmp/${KEY}.[RFO] /tmp/${KEY}
			fi
			if [ ! -d $INDIR ]
			then mkdir $INDIR; chmod 755 $INDIR
			fi
			if [ ! -d ${INDIR}/Files ]
			then mkdir ${INDIR}/Files; chmod 755 ${INDIR}/Files
			fi
			if [ ! -d ${INDIR}/Remove ]
			then mkdir ${INDIR}/Remove; chmod 755 ${INDIR}/Remove
			fi
			echo $KEY2 1 $NAME >> $CONTENTS
			INSTALLED_LIST="$INSTALLED_LIST $NAME"
			mv ./Remove ${INDIR}/Remove/${KEY2}
			mv ./Name /usr/options/${KEY2}
			if [ -f Files ]
			then
				mv Files ${INDIR}/Files/${KEY2}
			fi
			rm -rf ${TMPDIR} ${CPLOG}
			sync; sync
			message -d "The installation of the $NAME package is now complete."
		fi
	fi
done
rm -f /tmp/Ap$$
cd /; rm -rf /tmp/pkg.idx ${TMP_FILE} ${PACK_FLNM} ${TMPDIR} ${CPLOG}; sync

if [ -z "$SPECIAL" ]
then eval $REWIND
fi

if [ $ERROR -eq 1 ]
then

	echo "The Installation process is aborted."
	if [ ! -z "$INSTALLED_LIST" ] 
	then
		echo "The following packages have already been installed:\n"
		for i in $INSTALLED_LIST
		do
			echo "           \"`eval echo \\$PKG$i`\""
		done
		echo "\n\n"
		echo "Use 'removepkg' if you need to remove any of the installed packages"
	fi
	exit 1
fi
if [ -f /etc/.new_unix ]
then
sync; sync; cd /
exec /etc/conf/bin/idreboot
fi
