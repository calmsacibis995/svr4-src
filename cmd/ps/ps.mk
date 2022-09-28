#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)ps:ps.mk	1.7.14.1"
ROOT=
INC = $(ROOT)/usr/include
LDFLAGS = -s -lw
INS = install
CFLAGS = -O -I$(INC)
INSDIR = $(ROOT)/usr/bin

all:	ps

ps: ps.c
	$(CC) $(CFLAGS) -o ps ps.c $(LDFLAGS) $(PERFLIBS)

install: ps
	$(INS) -f $(INSDIR) -m 4555 -u root -g sys ps

clean:
	-rm -f ps.o

clobber: clean
	rm -f ps
