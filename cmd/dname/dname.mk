#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)dname:dname.mk	1.4.7.1"


ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/sbin
INC = $(ROOT)/usr/include
INS = install
SYMLINK = :
CFLAGS = -O -s
LDFLAGS = -lns $(SHLIBS)
FRC =

all: dname

dname: dname.c 
	$(CC) -I$(INC) $(CFLAGS) -o $(TESTDIR)/dname dname.c $(LDFLAGS)

install: all
	-rm -f $(ROOT)/usr/bin/dname
	$(INS) -f $(INSDIR) -m 555 -u bin -g bin dname
	-$(SYMLINK) /usr/sbin/dname $(ROOT)/usr/bin/dname

clean:
	rm -f *.o

clobber: clean
	rm -f $(TESTDIR)/dname
FRC:
