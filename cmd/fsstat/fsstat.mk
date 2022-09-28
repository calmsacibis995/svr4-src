#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)fsstat:fsstat.mk	1.9.1.1"
ROOT =
TESTDIR = .
INSDIR = $(ROOT)/sbin
INC = $(ROOT)/usr/include
INS = install
CFLAGS = -O
LDFLAGS = -s
SYMLINK = :
FRC =

all: fsstat

fsstat: fsstat.c  \
	$(INC)/stdio.h \
	$(INC)/fcntl.h \
	$(INC)/sys/param.h \
	$(INC)/sys/types.h \
	$(INC)/sys/fs/s5param.h \
	$(INC)/sys/fs/s5filsys.h \
	$(INC)/sys/stat.h \
	$(INC)/ustat.h
	$(CC) -I$(INC) $(CFLAGS) -o $(TESTDIR)/fsstat fsstat.c $(LDFLAGS) $(ROOTLIBS)

install: all
	-rm -f $(ROOT)/etc/fsstat
	$(INS) -f $(INSDIR) -m 0555 -u root -g sys $(TESTDIR)/fsstat
	$(INS) -f $(ROOT)/usr/sbin -m 0555 -u root -g sys $(TESTDIR)/fsstat
	-$(SYMLINK) /sbin/fsstat $(ROOT)/etc/fsstat

clean:

clobber: clean
	rm -f $(TESTDIR)/fsstat
FRC:
