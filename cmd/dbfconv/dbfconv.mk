#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)dbfconv:dbfconv.mk	1.6.1.1"

ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/lib/saf
INC = $(ROOT)/usr/include
INS = install
CFLAGS = -O -I$(INC)
LDFLAGS = -s
SOURCE = dbfconv.c nlsstr.c
OBJECT = dbfconv.o nlsstr.o
FRC =

all: dbfconv

dbfconv: $(OBJECT)
	$(CC) $(CFLAGS) -o $(TESTDIR)/dbfconv $(LDFLAGS) $(OBJECT) $(SHLIBS)

$(INSDIR):
	mkdir $@

install: all $(INSDIR)
	$(INS) -f $(INSDIR) -m 0755 -u root -g sys $(TESTDIR)/dbfconv

clean:
	rm -f $(OBJECT)

clobber: clean
	rm -f $(TESTDIR)/dbfconv

FRC:


dbfconv.o: $(INC)/sys/param.h \
	$(INC)/sys/fs/s5dir.h \
	local.h \
	$(INC)/stdio.h
