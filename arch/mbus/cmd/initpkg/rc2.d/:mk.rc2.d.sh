#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)mbus:cmd/initpkg/rc2.d/:mk.rc2.d.sh	1.3.1.1"

STARTLST=" 01MOUNTFSYS 05RMTMPFILES 06bootserver 15mkdtab 20sysetup 41mb2_init 70uucp 75cron"

STOPLST=

INSDIR=${ROOT}/etc/rc2.d
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
	eval ${CH}chmod 744 ${INSDIR}/K$f
	eval ${CH}chgrp sys ${INSDIR}/K$f
	eval ${CH}chown root ${INSDIR}/K$f
done
for f in ${STARTLST}
do 
	name=`echo $f | sed -e 's/^..//'`
	rm -f ${INSDIR}/S$f
	ln ${ROOT}/etc/init.d/${name} ${INSDIR}/S$f
	eval ${CH}chmod 744 ${INSDIR}/S$f
	eval ${CH}chgrp sys ${INSDIR}/S$f
	eval ${CH}chown root ${INSDIR}/S$f
done
