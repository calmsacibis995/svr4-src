#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)mvdir:mvdir.mk	1.5.1.1"

ROOT =
SYMLINK = :
INS = install

all:	mvdir.sh
	cp mvdir.sh mvdir

install:	all
	-rm -f $(ROOT)/etc/mvdir
	$(INS) -f $(ROOT)/usr/sbin -m 0544 -u root -g bin mvdir
	-$(SYMLINK) /usr/sbin/mvdir $(ROOT)/etc/mvdir

clean:
	rm -f mvdir

clobber:	clean

