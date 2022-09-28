#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)fdetach:fdetach.mk	1.1.3.1"

#	fdetach make file

ROOT=
INC = $(ROOT)/usr/include
INSDIR = $(ROOT)/usr/bin
CFLAGS = -O -c -I$(INC)
LDFLAGS = -s
INS=/etc/install

all:	fdetach

fdetach: fdetach.o
	$(CC) -o fdetach $(LDFLAGS) fdetach.o
	$(CH)chmod 755 fdetach

fdetach.o: fdetach.c \
	$(INC)/stdio.h
	$(CC) $(CFLAGS) fdetach.c

install: fdetach
	$(INS) -f $(INSDIR) -m 755 -u bin -g bin fdetach

clean:
	-rm -f fdetach.o

clobber: clean
	rm -f fdetach

