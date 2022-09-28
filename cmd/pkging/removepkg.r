#!/sbin/sh
#
#ident	"@(#)pkging:removepkg.r	1.2"
#
# PURPOSE: Delete installed software
# ----------------------------------------------------------------------
PATH=/sbin:/usr/sbin:/etc:/usr/bin
export PATH
FILE=/tmp/R$$
trap 'rm -rf ${FILE}; cd /; /sbin/umount /install > /dev/null 2>&1; exit 1' 1 2 3 9 15

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
then echo "You must be root or super-user to remove software."
     exit 1
fi

if [ -z "$TERM" ]
then TERM=AT386-M
fi
CLEAR=`tput clear 2>/dev/null`
unset ROOT

ERROR5="There are currently no software applications installed that can be removed."

CONTENTS=/usr/lib/installed/CONTENTS
INDIR=/usr/lib/installed

if [ ! -d /usr/options ]
then
	echo "$ERROR5"
	exit 1
fi
cd /usr/options

if [ $# = 0 ]
then
	list=`ls *.name 2>/dev/null`
	if [ -z "$list" ]
	then
		echo "$ERROR5"
		exit 1
	fi
	for i in *
	do
		grep "NAME=`cat $i`" /var/sadm/pkg/*/pkginfo > /dev/null 2>&1
		if [ $? = 0 ]
		then continue
		fi
		if [ -h $i ]
		then continue
		fi
		echo "$i\t\c"
		cat $i | sed -e "/^$/d"
	done > ${FILE}
	max=`expr \`cat $FILE | wc -l\``
	if [ $max -lt 1 ]
	then
		echo "$ERROR5"
		exit 1
	fi
	while [ 1 ]
	do
		pr -n -l1 -t ${FILE} | cut -d'	' -f1,3-
		max=`expr \`cat $FILE | wc -l\``
		echo "\nSelect a number (1 - $max) from this list to remove: \c"
		read index
		expr $index \* 20 > /dev/null 2>&1
		if [ $? != 0 ]
		then echo "That number is invalid.  Please specify a number between 1 and $max."
     		continue
		elif [ $index -gt $max ]
		then echo "That number is too large.  Please specify a number between 1 and $max."
     		continue
		elif [ $index -lt 1 ]
		then echo "That number is too small.  Please specify a number between 1 and $max."
     		continue
		else break
		fi
	done
	PROG=`sed -n "${index}p" $FILE | cut -d'	' -f1 | sed -e "s/\.name//"`
	rm -rf ${FILE}
else
	NAME=`echo ${1} | tr "[A-Z]" "[a-z]"`
	if [ -f ${NAME}.name ]
	then PROG=$NAME
	else
		for i in *
		do
			grep -i "$NAME" $i > /dev/null 2>&1
			if [ $? = 0 ]
			then PROG=`echo $i | sed -e 's/\.name//'`
			     break
			fi
		done
		if [ -z "$PROG" ]
		then echo "\nThere is no software package currently installed\nresembling: $1\n"
		     exit 1
		fi
	fi
	grep "NAME=`cat ${PROG}.name`" /var/sadm/pkg/*/pkginfo > /dev/null 2>&1
	if [ $? = 0 ] || [ -h ${PROG}.name ]
	then
		message -d "The package $1 is an OA&M style package.  To remove this \
package, type in:\n\t\tpkgrm ${PROG}"
		exit 1
	fi
fi

cd /
KEY=${PROG}.name
NAME=`cat /usr/options/${PROG}.name`
grep $PROG $CONTENTS > /dev/null 2>&1
if [ $? = 0 ]
then

message -c "Do you really want to remove $NAME?"
if [ "$?" != "0" ]; then exit; fi
	trap '' 1 2 3 9 15
export KEY NAME

UNFILE=${INDIR}/Remove/"${KEY}"
if [ -r "$UNFILE" ]
then
	chmod +x "$UNFILE"
	echo "$CLEAR "
	"$UNFILE"
	if [ $? = "0" ]
	then
		sync; sync
		grep -v "^$KEY " $CONTENTS > /tmp/$$
		mv /tmp/$$ $CONTENTS
		rm "$UNFILE"
		rm -f ${INDIR}/Files/"${KEY}" /usr/options/"${KEY}"
		sync; sync; sync
		echo "$CLEAR "
		if [ -f /etc/.new_unix ]
		then
			sync; sync
			exec /etc/conf/bin/idreboot
		else
			message -d "The $NAME is now removed."
		fi
		exit
	fi
	exit
else
	if [ -f ${INDIR}/Files/"${KEY}" ]
	then FILES_MESS="  The file ${INDIR}/Files/${KEY} contains a list of the \
files and directories installed or created by the package.  You may wish to use this \
file to help in removing the package."
	fi
	message -d "Cannot find removal script for $NAME.  You will have to \
remove this package manually using UNIX System tools from the UNIX System Shell.${FILES_MESS}"
fi
else

/sbin/mount | grep "^/install" > /dev/null 2>&1
if [ $? = 0 ]
then
	message -d "Cannot remove ${NAME}.  The /install directory is currently \
mounted.  Please unmount /install and then try again."
	exit 1
fi

MES1=
MSG=
/sbin/flop_num
if [ $? = 2 ]
then
	while true
	do
		echo "\nThis system has two floppy drives.\n\
Strike ENTER to remove from drive 0\n\
or 1 to remove from drive 1.  \c"
		read ans
		if [ "$ans" = 1 ]
		then
			DRIVE=/dev/dsk/f1
			break
		elif [ "$ans" = "" -o "$ans" = 0 ]
		then
			DRIVE=/dev/dsk/f0
			break
		fi
	done
else
	DRIVE=/dev/dsk/f0
fi
while [ 1 ]
do
message -c "${MSG}Insert the removable medium for the $NAME you wish to remove."
if [ "$?" != "0" ]; then exit; fi
/sbin/mount $DRIVE /install -r > /dev/null 2>&1
if [ $? = 0 ]
then 
     /sbin/umount /install > /dev/null 2>&1
     break
fi

/sbin/mount ${DRIVE}t /install -r > /dev/null 2>&1
if [ $? = 0 ]
then 
     /sbin/umount /install > /dev/null 2>&1
     DRIVE=${DRIVE}t
     break
fi
/sbin/umount /install > /dev/null 2>&1

MSG="Cannot determine the type of floppy.\n"
done
/sbin/mount ${DRIVE} /install -r > /dev/null 2>&1
if [ ! -r /install/install/UNINSTALL ]
then echo >&2 '\tThis package will not remove itself.  Consult the instructions \
that came with the medium.'
     /sbin/umount /install > /dev/null 2>&1
fi
trap "trap '' 1 2 3 9 15; cd /; rm -rf /tmp/$$UNINSTALL; /sbin/umount /install; exit 1" 1 2 3
cd /tmp
cp /install/install/UNINSTALL $$UNINSTALL
chmod +x $$UNINSTALL
/tmp/$$UNINSTALL ${DRIVE} /install "`basename ${DRIVE}` drive" ||
	echo 'WARNING:  Package removal may not have completed properly.'
trap '' 1 2 3
cd /
rm -rf /tmp/$$UNINSTALL
/sbin/umount /install > /dev/null 2>&1
if [ -f /etc/.new_unix ]
then
	sync; sync
	exec /etc/conf/bin/idreboot
fi
fi
