#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)curses:screen/rmident.sh	1.5"
H='curses.ext maketerm.ed compiler.h curses.form curshdr.h object.h otermcap.h print.h unctrl.h'
for i in $H
do 
	echo de-identing $i
	mv $i zzy_$i
	sed '/ident/d' < zzy_$i > $i
	rm -f zzy_$i
done
