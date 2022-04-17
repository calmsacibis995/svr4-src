#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)adv:adv.mk	1.4.7.1"


ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/sbin
INC = $(ROOT)/usr/include
SYMLINK = :
LDFLAGS = -lns
INS = install
CFLAGS = -O -s
FRC =

all: adv

adv: adv.c 
	$(CC) -I$(INC) $(CFLAGS) -o $(TESTDIR)/adv adv.c $(LDFLAGS) $(SHLIBS)

install: all
	-rm -f $(ROOT)/usr/bin/adv
	$(INS) -f $(INSDIR) -m 555 -u bin -g bin adv
	-$(SYMLINK) /usr/sbin/adv $(ROOT)/usr/bin/adv

clean:
	rm -f adv.o

clobber: clean
	rm -f $(TESTDIR)/adv
FRC:
