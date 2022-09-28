#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)nlsadmin:nlsadmin.mk	1.5.5.1"

ROOT =
TESTDIR = .
INSDIR = $(ROOT)/usr/sbin
INC = $(ROOT)/usr/include
INCSYS = $(ROOT)/usr/include
INS = install
CFLAGS = -O -I$(INC) -I$(INCSYS)
LDFLAGS = -s
SOURCE = nlsadmin.c 
OBJECTS = nlsadmin.o 
FRC =

all: nlsadmin

nlsadmin: $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TESTDIR)/nlsadmin $(LDFLAGS) $(OBJECTS) $(SHLIBS)

install: all
	$(INS) -f $(INSDIR) -m 0755 -u root -g adm $(TESTDIR)/nlsadmin

clean:
	rm -f $(OBJECTS)

clobber: clean
	rm -f $(TESTDIR)/nlsadmin
FRC:


nlsadmin.o: $(INCSYS)/sys/types.h \
	$(INCSYS)/sys/stat.h \
	$(INC)/stdio.h \
	$(INC)/ctype.h \
	$(INC)/string.h \
	$(INC)/errno.h \
	$(INC)/sac.h \
	nlsadmin.h 
