#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)oamintf:files/bin/isoam.rem.sh	1.1.1.2"

# Determine if a package is a removable OAM style package; RC=0 means yes

#
# Test for existence of 4.0 /var/sadm/pkg/<pkgname>
#

OAMSTYLE=""
OTHSTYLE=""
TESTDEV=`echo $2 | cut -f2 -d" "`

for NAME in `echo "${1}" | tr "," " "`
do
	if [ "$TESTDEV" ] && [ -d /usr/"$TESTDEV"/pkg/$NAME ]
	then
		OAMSTYLE="${OAMSTYLE} ${NAME}"
	elif [ -d /var/sadm/pkg/${NAME} ]
	then
		OAMSTYLE="${OAMSTYLE} ${NAME}"
	else
		OTHSTYLE="${OTHSTYLE} ${NAME}"
	fi

done

if [ ! -z "${OAMSTYLE}" ]
then
	pkgrm ${2} ${OAMSTYLE}
fi

if [ ! -z "${OTHSTYLE}" ]
then
	for PKG in ${OTHSTYLE}
	do
		removepkg ${PKG}
	done
fi

exit 0
