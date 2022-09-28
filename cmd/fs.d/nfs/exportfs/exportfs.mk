#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nfs.cmds:nfs/exportfs/exportfs.mk	1.11.2.1"
INS = install
INSDIR = $(ROOT)/usr/lib/nfs
SYMLINK = :

all: exportfs.sh
	cp exportfs.sh exportfs

install: all
	if [ ! -d $(INSDIR) ] ; \
	then mkdir -p $(INSDIR); \
	fi
	if [ ! -d $(ROOT)/etc/nfs ] ; \
	then mkdir -p $(ROOT)/etc/nfs ; \
	fi
	$(INS) -f $(INSDIR) exportfs
	-rm -f $(ROOT)/etc/nfs/exportfs
	-$(SYMLINK) /usr/lib/nfs/exportfs $(ROOT)/etc/nfs/exportfs

lint:

tags:

clean:

clobber: clean
	-rm -f exportfs
