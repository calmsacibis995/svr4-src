#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)install:install.mk	1.2.2.1"

ROOT =
SYMLINK = :
INS = install

all:	install

install:
	cp install.sh  install
	$(INS) -o -f $(ROOT)/usr/sbin -m 0555 -u bin -g bin install
	-$(SYMLINK) /usr/sbin/install $(ROOT)/etc/install

clean:

clobber:	clean
	-rm -rf install
	-rm -rf $(ROOT)/usr/sbin/OLDinstall
