#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)rfuadmin:rfuadmin.mk	1.3.6.1"



#	rfuadmin make file

ROOT=
SYMLINK = :
INSDIR = $(ROOT)/etc/rfs
INS=install

all:	install clobber

rfuadmin:
	sh rfuadmin.sh

install: rfuadmin
	-@if [ ! -d "$(INSDIR)" ] ; \
	then \
		mkdir $(INSDIR) ; \
	fi ;
	-rm -f $(ROOT)/usr/nserve/rfuadmin
	$(INS) -f $(INSDIR) -m 554 -u bin -g bin rfuadmin
	-@if [ ! -d "$(ROOT)/usr/nserve" ] ; \
	then \
		mkdir $(ROOT)/usr/nserve ; \
	fi ;
	-$(SYMLINK) /etc/rfs/rfuadmin $(ROOT)/usr/nserve/rfuadmin

clean:
	rm -f rfuadmin

clobber: clean
