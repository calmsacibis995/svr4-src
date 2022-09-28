#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)face:src/vinstall/vsetup.sh	1.7"
ferror()
{
	echo $1 ; exit 1
}
set -a

LOGINID=${1}
service=`echo ${2}|cut -c1`
autoface=`echo ${3}|cut -c1`
shell_esc=`echo ${4}|cut -c1`

LOGDIR=`sed -n -e "/^${LOGINID}:/s/^.*:\([^:][^:]*\):[^:]*$/\1/p" < /etc/passwd`
if [ ! -d "${LOGDIR}" ]
then
	echo "${LOGINID}'s home directory doesn't exist"
	exit 1
fi

VMSYS=`sed -n -e '/^vmsys:/s/^.*:\([^:][^:]*\):[^:]*$/\1/p' < /etc/passwd`
if [ ! -d "${VMSYS}" ]
then
	echo "The value for VMSYS is not set."
	exit 1
fi

UHOME=`grep -s "^$LOGINID:" /etc/passwd | cut -f6 -d:`
if [ -z "${UHOME}" ]
then
	echo "\n${LOGNID}'s home directory has not been retrieved correctly."
	exit 1
fi

GRPNAME=`grep -s "^$LOGINID:" /etc/passwd 2> /dev/null | cut -f4 -d: `
if [ -z "${GRPNAME}" ]
then 
	echo "\n$LOGINID's group is not in /etc/group.\n"
	exit 1
fi

cp $VMSYS/standard/.faceprofile ${UHOME} || ferror "Can't access $LOGINID's home directory"
chmod 644 ${UHOME}/.faceprofile || ferror "Can't access $LOGINID's home directory"
chown  ${LOGINID} ${UHOME}/.faceprofile || ferror "Can't access $LOGINID's home directory"
chgrp ${GRPNAME} ${UHOME}/.faceprofile || ferror "Can't access $LOGINID's home directory"


cd ${UHOME}     
for dir in WASTEBASKET pref tmp bin
do
	if [ ! -d ${dir} ]
	then
		mkdir ${dir} || ferror "Can't create $dir in  $LOGINID's home directory"
		echo "\t${dir} directory has been created for ${LOGINID}"
		chgrp ${GRPNAME} ${dir}
		chown ${LOGINID} ${dir}
		chmod 755 ${dir}
	else
		echo "\t${dir} directory already exists"
	fi
done

if [ ! -f ${UHOME}/pref/services ]
then
	echo '#3B2-4I1' > ${UHOME}/pref/services
	chmod 644 ${UHOME}/pref/services
	chown  ${LOGINID} ${UHOME}/pref/services
	chgrp ${GRPNAME} ${UHOME}/pref/services
fi

if [ ! -f ${UHOME}/.profile ]
then
	if [ -f /etc/stdprofile ]
	then
		cp /etc/stdprofile ${UHOME}/.profile
	else
		touch ${UHOME}/.profile
	fi
	chmod 644 ${UHOME}/.profile
	chown  ${LOGINID} ${UHOME}/.profile
	chgrp ${GRPNAME} ${UHOME}/.profile
fi

$VMSYS/bin/chkperm -${autoface} invoke -u ${LOGINID} 2>&1 || ferror "You must be super-user to set the FACE permissions for $LOGINID."

$VMSYS/bin/chkperm -${service} admin -u ${LOGINID} 2>&1 || ferror "You must be super-user to set the FACE permissions for $LOGINID."

$VMSYS/bin/chkperm -${shell_esc} unix -u ${LOGINID} 2>&1 || ferror "You must be super-user to set the FACE permissions for $LOGINID."

if grep '^\. \$HOME/\.faceprofile$' ${UHOME}/.profile > /dev/null
then
	exit 0
else
	echo '. $HOME/.faceprofile' >> ${UHOME}/.profile
fi

exit 0
