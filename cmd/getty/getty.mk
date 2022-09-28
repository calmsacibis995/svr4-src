#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)getty:getty.mk	1.5.4.3"

TESTDIR = .
FRC =
INS = install
INSDIR = $(ROOT)/sbin
DFLGS = -DMERGE386
CFLAGS= -O -DSYS_NAME $(DFLGS)

LDFLAGS = -s -n

all: getty

getty: getty.c $(FRC)
	$(CC) $(CFLAGS) -o $(TESTDIR)/getty getty.c $(LDFLAGS) $(SHLIBS)

test:
	rtest $(TESTDIR)/getty

install: all
	$(INS) -f $(INSDIR) -o $(TESTDIR)/getty $(INSDIR)

clean:

clobber: clean
	-rm -f $(TESTDIR)/getty

FRC:
