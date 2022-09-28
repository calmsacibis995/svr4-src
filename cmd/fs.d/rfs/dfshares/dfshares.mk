#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)rfs.cmds:dfshares/dfshares.mk	1.2.3.1"


ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/lib/fs/rfs
INC = $(ROOT)/usr/include
LDFLAGS = -lns $(SHLIBS)
INS = install
CFLAGS = -O -s
FRC =

all: dfshares

dfshares: dfshares.c 
	$(CC) -I$(INC) $(CFLAGS) -o $(TESTDIR)/dfshares dfshares.c $(LDFLAGS)

install: all
	@if [ ! -d $(INSDIR) ]; then mkdir -p $(INSDIR); fi;
	$(INS) -f $(INSDIR) -m 04555 -u root -g bin dfshares

clean:
	rm -f dfshares.o

clobber: clean
	rm -f $(TESTDIR)/dfshares
FRC:
