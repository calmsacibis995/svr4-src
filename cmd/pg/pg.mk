#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)pg:pg.mk	1.12"

ROOT =
TESTDIR = .
INS = install
INC = $(ROOT)/usr/include
LIBCURSES=-lcurses
CFLAGS = -O -I$(INC)
LDFLAGS = -s -lgen -lw

all: pg

pg: pg.c
	$(CC) $(CFLAGS) -o $(TESTDIR)/pg pg.c $(LDFLAGS) $(LIBCURSES) $(PERFLIBS)

install: all
	$(INS) -f $(ROOT)/usr/bin -m 0555 -u bin -g bin $(TESTDIR)/pg

clean:
	-rm -f pg.o

clobber: clean
	-rm -f $(TESTDIR)/pg
