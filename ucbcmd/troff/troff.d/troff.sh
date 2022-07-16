#!/sbin/sh
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)ucbtroff:troff.d/troff.sh	1.1.2.1"
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
#	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
#	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
#	          All rights reserved.



dev=aps
oflags= newargs=

for i
do
	case $i in
	-Tcat)	dev=cat ;;
	-Taps)	dev=aps ;;
	-T*)	echo invalid option $i;  exit 1 ;;
	-c*)	cm=`echo $i|sed -e s/c/m/`
		newargs="$newargs $cm"
		oflags=$oflags$i  ;;
	-b|-k*|-p*|-g|-w)	oflags=$i$oflags ;;
	*)	newargs="$newargs $i"  ;;
	esac
done

case $dev in

cat)
	exec otroff $*
	;;

aps)
	if [ "-b" = "$oflags" ]
	then
		echo '-b no longer supported'
		exit 1
	fi
	if [ -n "$oflags" ]
	then
		echo "options -c, -k, -b, -w, -g and -p are no longer supported"
	fi
	exec troff $newargs
	;;

esac
