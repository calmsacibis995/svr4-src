#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)face:src/vinstall/vmodify.sh	1.1"
ferror()
{
	echo $1 ; exit 1
}
set -a

LOGINID=${1}
service=`echo ${2}|cut -c1`
autoface=`echo ${3}|cut -c1`
shell_esc=`echo ${4}|cut -c1`

VMSYS=`sed -n -e '/^vmsys:/s/^.*:\([^:][^:]*\):[^:]*$/\1/p' < /etc/passwd`
if [ ! -d "${VMSYS}" ]
then
	echo "The value for VMSYS is not set."
	exit 1
fi

$VMSYS/bin/chkperm -${autoface} invoke -u ${LOGINID} 2>&1 || ferror "You must be super-user to set the FACE permissions for $LOGINID."

$VMSYS/bin/chkperm -${service} admin -u ${LOGINID} 2>&1 || ferror "You must be super-user to set the FACE permissions for $LOGINID."

$VMSYS/bin/chkperm -${shell_esc} unix -u ${LOGINID} 2>&1 || ferror "You must be super-user to set the FACE permissions for $LOGINID."

exit 0
