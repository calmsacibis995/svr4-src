#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)rfpasswd:rfpasswd.mk	1.9.5.1"


ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/sbin
INC = $(ROOT)/usr/include
INS = install
SYMLINK = :
CFLAGS = -O -s
LDFLAGS = -lns -lcrypt_i $(SHLIBS)
FRC =

all: rfpasswd

rfpasswd: rfpasswd.c 
	$(CC) -I$(INC) $(CFLAGS) -o $(TESTDIR)/rfpasswd rfpasswd.c $(LDFLAGS)

install: all
	-rm -f $(ROOT)/usr/bin/rfpasswd
	$(INS) -f $(INSDIR) -m 555 -u bin -g bin rfpasswd
	-$(SYMLINK) /usr/sbin/rfpasswd $(ROOT)/usr/bin/rfpasswd

clean:
	rm -f rfpasswd.o

clobber: clean
	rm -f $(TESTDIR)/rfpasswd
FRC:
