#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)file:file.mk	1.14"
#
ROOT =
TESTDIR=.
INS = install
INSDIR = $(ROOT)/usr/bin
MINSDIR = $(ROOT)/etc
INC = $(ROOT)/usr/include
CFLAGS = -O -I$(INC)
LDFLAGS = -s -lcmd  -lw 

all:	file

file:	file.c
	$(CC) $(CFLAGS) file.c -o $(TESTDIR)/file $(LDFLAGS) $(SHLIBS)

install: all
	$(INS) -f $(INSDIR) -m 0555 -u bin -g bin $(TESTDIR)/file
	$(INS) -f $(MINSDIR) -m 0444 -u bin -g bin magic

clean:
	-rm -f file.o

clobber: clean
	-rm -f $(TESTDIR)/file
