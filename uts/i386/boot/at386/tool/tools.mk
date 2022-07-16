#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1987, 1988 Microsoft Corporation
#	  All Rights Reserved

#	This Module contains Proprietary Information of Microsoft
#	Corporation and should be treated as Confidential.

#ident	"@(#)boot:boot/at386/tool/tools.mk	1.1.2.1"


all:  rmhdr setfilter

clean:	
	-/bin/rm rmhdr.o
	-/bin/rm setfilter.o

clobber: clean
	-/bin/rm rmhdr
	-/bin/rm setfilter

rmhdr:
	PATH=/usr/ccs/bin:/bin:${PATH} \
		cc -O -s -I${INCRT} -o rmhdr rmhdr.c

setfilter:
	PATH=/usr/ccs/bin:/bin:${PATH} \
		cc -O -s -I${INCRT} -o setfilter setfilter.c

install:

