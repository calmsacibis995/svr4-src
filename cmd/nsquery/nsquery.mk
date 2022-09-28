#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)nsquery:nsquery.mk	1.5.7.1"


ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/sbin
INC = $(ROOT)/usr/include
SYMLINK = :
LDFLAGS = -lns $(SHLIBS)
INS = install
CFLAGS = -O -s
FRC =

all: nsquery

nsquery: nsquery.c 
	$(CC) -I$(INC) $(CFLAGS) -o $(TESTDIR)/nsquery nsquery.c $(LDFLAGS)

install: all
	-rm -f $(ROOT)/usr/bin/nsquery
	$(INS) -f $(INSDIR) -m 04555 -u root -g bin nsquery
	-$(SYMLINK) $(INSDIR)/nsquery $(ROOT)/usr/bin/nsquery

clean:
	rm -f nsquery.o

clobber: clean
	rm -f $(TESTDIR)/nsquery
FRC:
