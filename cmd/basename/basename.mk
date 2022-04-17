#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)basename:basename.mk	1.3.1.1"

ROOT =
SYMLINK = :
INS = install

all:	basename.sh
	cp basename.sh  basename

install:	all
	$(INS) -f $(ROOT)/usr/bin -m 0555 -u bin -g bin basename

clean:

clobber:	clean
	rm -f basename
