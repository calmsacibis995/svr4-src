#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)face:src/vinstall/vdelete.sh	1.1"
ferror()
{
	echo $1 ; exit 1
}
set -a

LOGINID=${1}

VMSYS=`sed -n -e '/^vmsys:/s/^.*:\([^:][^:]*\):[^:]*$/\1/p' /etc/passwd`
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

$VMSYS/bin/chkperm -d -u ${LOGINID} 2>&1 || ferror "You must be super-user to remove $LOGINID as a FACE user."

if grep '^\. \$HOME/\.faceprofile$' ${UHOME}/.profile > /dev/null
then
	grep -v '^\. \$HOME/\.faceprofile$' ${UHOME}/.profile > /tmp/f.del.$$
	cp /tmp/f.del.$$ ${UHOME}/.profile
fi

exit 0
