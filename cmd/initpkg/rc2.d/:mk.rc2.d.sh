#!/sbin.sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)initpkg:./rc2.d/:mk.rc2.d.sh	1.12.10.1"

if i386
then
	STARTLST="01MOUNTFSYS 05RMTMPFILES \
	15mkdtab 20sysetup 70uucp 75cron 95osm"
else
	STARTLST=" 00firstcheck 01MOUNTFSYS 05RMTMPFILES 08ports 10disks \
	15mkdtab 20sysetup 70uucp 75cron"
fi

STOPLST="30fumounts 40rumounts 50rfs"

INSDIR=${ROOT}/etc/rc2.d
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
	for f in ${STARTLST}
	do 
		name=`echo $f | sed -e 's/^..//'`
		rm -f ${INSDIR}/S$f
		ln ${ROOT}/etc/init.d/${name} ${INSDIR}/S$f
	done
fi
