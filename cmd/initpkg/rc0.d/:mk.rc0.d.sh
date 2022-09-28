#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)initpkg:./rc0.d/:mk.rc0.d.sh	1.7.7.1"

STARTLST= 
STOPLST="00ANNOUNCE 50fumounts 60rumounts 65rfs 70cron"

INSDIR=${ROOT}/etc/rc0.d
if u3b2 || i386
then
	if [ ! -d ${INSDIR} ] 
	then 
		mkdir ${INSDIR} 
		eval ${CH}chmod 755 ${INSDIR}
		eval ${CH}chgrp sys ${INSDIR}
		eval ${CH}chown root ${INSDIR}
	fi 
	for f in ${STOPLST}
	do 
		name=`echo $f | sed -e 's/^..//'`
		rm -f ${INSDIR}/K$f
		ln ${ROOT}/etc/init.d/${name} ${INSDIR}/K$f
	done
fi
