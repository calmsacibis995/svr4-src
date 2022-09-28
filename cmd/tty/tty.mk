#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)tty:tty.mk	1.4.1.1"
#	tty make file

ROOT =
INSDIR = $(ROOT)/usr/bin
INS = install
CFLAGS = -O $(FFLAG)
LDFLAGS = -s $(IFLAG)
SOURCE = tty.c
MAKE = make

all:	$(SOURCE)
	$(CC) $(CFLAGS) $(LDFLAGS) -o tty tty.c -lm $(SHLIBS)

install: all
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin tty

clean:
	rm -f tty.o

clobber:	clean
	  rm -f tty
