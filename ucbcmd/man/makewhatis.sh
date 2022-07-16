#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)ucbman:makewhatis.sh	1.2.4.1"
#		PROPRIETARY NOTICE (Combined)
#
#This source code is unpublished proprietary information
#constituting, or derived under license from AT&T's UNIX(r) System V.
#In addition, portions of such source code were derived from Berkeley
#4.3 BSD under license from the Regents of the University of
#California.
#
#
#
#		Copyright Notice 
#
#Notice of copyright on this source code product does not indicate 
#publication.
#
#	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
#	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
#	          All rights reserved.

trap "rm -f /tmp/whatisx.$$ /tmp/whatis$$; exit 1" 1 2 13 15
MANDIR=${1-/usr/share/man}
rm -f /tmp/whatisx.$$ /tmp/whatis$$
if test ! -d $MANDIR ; then exit 0 ; fi
cd $MANDIR
top=`pwd`
for i in man1 man2 man3 man4 man5 man6 man7 man8 mann manl
do
	if [ -d $i ] ; then
		cd $i
	 	if test "`echo *.*`" != "*.*" ; then
			/usr/ucblib/getNAME *.*
		fi
		cd $top
	fi
done >/tmp/whatisx.$$
sed  </tmp/whatisx.$$ >/tmp/whatis$$ \
	-e 's/\\-/-/' \
	-e 's/\\\*-/-/' \
	-e 's/ VAX-11//' \
	-e 's/\\f[PRIB0123]//g' \
	-e 's/\\s[-+0-9]*//g' \
	-e 's/.TH [^ ]* \([^ 	]*\).*	\([^-]*\)/\2(\1)	/' \
	-e 's/	 /	/g'
/usr/ucb/expand -24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,84,88,92,96,100 \
	/tmp/whatis$$ | sort | /usr/ucb/unexpand -a > whatis
chmod 644 whatis >/dev/null 2>&1
rm -f /tmp/whatisx.$$ /tmp/whatis$$
exit 0
