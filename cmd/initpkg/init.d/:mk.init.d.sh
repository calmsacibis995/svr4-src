#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)initpkg:init.d/:mk.init.d.sh	1.2.6.1"

INSDIR=${ROOT}/etc/init.d
INS=${INS:-install}
if u3b2 || i386
then
	if [ ! -d ${INSDIR} ] 
	then 
		mkdir ${INSDIR} 
		if [ $? != 0 ]
		then
			exit 1
		fi
		eval ${CH}chmod 755 ${INSDIR}
		eval ${CH}chgrp sys ${INSDIR}
		eval ${CH}chown root ${INSDIR}
	fi 
	for f in [a-zA-Z0-9]* 
	do 
		eval ${INS} -f ${INSDIR} -m 0744 -u root -g sys $f
	done
fi
