#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)unadv:unadv.mk	1.5.6.1"


ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/sbin
INC = $(ROOT)/usr/include
SYMLINK = :
LDFLAGS = -lns $(SHLIBS)
INS = install
CFLAGS = -O -s
FRC =

all: unadv

unadv: unadv.c 
	$(CC) -I$(INC) $(CFLAGS) -o $(TESTDIR)/unadv unadv.c $(LDFLAGS)

install: all
	-rm -f $(ROOT)/usr/bin/unadv
	$(INS) -f $(INSDIR) -m 555 -u bin -g bin unadv
	-$(SYMLINK) /usr/sbin/unadv $(ROOT)/usr/bin/unadv

clean:
	rm -f unadv.o

clobber: clean
	rm -f $(TESTDIR)/unadv
FRC:
