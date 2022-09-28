#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)rfadmin:rfadmin.mk	1.2.8.1"


ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/sbin
INC = $(ROOT)/usr/include
INS = install
SYMLINK = :
CFLAGS = -O -s
LDFLAGS = -lns -lcrypt_i $(SHLIBS)
FRC =

all: rfadmin

rfadmin: rfadmin.c 
	$(CC) -I$(INC) $(CFLAGS) -o $(TESTDIR)/rfadmin rfadmin.c $(LDFLAGS)

install: all
	-rm -f $(ROOT)/usr/bin/rfadmin
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin rfadmin
	-$(SYMLINK) /usr/sbin/rfadmin $(ROOT)/usr/bin/rfadmin

clean:
	rm -f rfadmin.o

clobber: clean
	rm -f $(TESTDIR)/rfadmin
FRC:
