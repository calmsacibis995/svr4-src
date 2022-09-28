#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)dustart:dustart.mk	1.7.5.1"


ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/sbin
INC = $(ROOT)/usr/include
INS = install
SYMLINK = :
CFLAGS = -O -s
LDFLAGS = -lns $(SHLIBS)
FRC =

all: rfstart

rfstart: rfstart.c 
	$(CC) -I$(INC) $(CFLAGS) -o $(TESTDIR)/rfstart rfstart.c $(LDFLAGS)

install: all
	-rm -f $(ROOT)/usr/bin/rfstart
	$(INS) -f $(INSDIR) -m 555 -u bin -g bin rfstart
	-$(SYMLINK) /usr/sbin/rfstart $(ROOT)/usr/bin/rfstart

clean:
	rm -f rfstart.o

clobber: clean
	rm -f $(TESTDIR)/rfstart
FRC:
