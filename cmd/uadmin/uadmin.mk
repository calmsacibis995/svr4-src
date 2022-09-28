#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)uadmin:uadmin.mk	1.10.1.1"
ROOT =
TESTDIR = .
INSDIR = $(ROOT)/sbin
INC = $(ROOT)/usr/include
INS = install
SYMLINK = :
CFLAGS = -O
LDFLAGS = -s 
FRC =

all: uadmin

uadmin: uadmin.c 
	$(CC) -I$(INC) $(CFLAGS) -o $(TESTDIR)/uadmin uadmin.c $(LDFLAGS) $(ROOTLIBS)

install: all
	-rm -f $(ROOT)/etc/uadmin
	-rm -f $(ROOT)/usr/sbin/uadmin
	$(INS) -f $(INSDIR) -m 0555 -u root -g sys $(TESTDIR)/uadmin
	$(INS) -f  $(ROOT)/usr/sbin -m 0555 -u root -g sys $(TESTDIR)/uadmin
	-$(SYMLINK) /sbin/uadmin $(ROOT)/etc/uadmin

clean:
	rm -f *.o

clobber: clean
	rm -f $(TESTDIR)/uadmin
FRC:
