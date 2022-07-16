#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)at:cmd/fdisk/fdisk.mk	1.3"

#       Makefile for fdisk

ROOT=

DIR = $(ROOT)/sbin

INC=$(ROOT)/usr/include

LDFLAGS = -s $(ROOTLIBS)

CFLAGS= -O -I$(INC)

STRIP = strip

SIZE = size

MAKEFILE = fdisk.mk

MAINS = fdisk

OBJECTS =  fdisk.o

SOURCES =  fdisk.c

ALL:		$(MAINS)

fdisk.o:	fdisk.c
	$(CC) $(CFLAGS) -c fdisk.c

fdisk:	fdisk.o
	$(CC) $(LDFLAGS) -o ./fdisk fdisk.o -lcurses

fdisk.c:            \
		 $(INC)/sys/types.h \
		 $(INC)/sys/vtoc.h \
		 $(INC)/sys/hd.h \
		 $(INC)/sys/fdisk.h \
		 $(INC)/curses.h \
		 $(INC)/term.h \
		 $(INC)/fcntl.h

clean:
	rm -f $(OBJECTS)

clobber:        clean
	rm -f fdisk

all : ALL

install: ALL
	cpset $(MAINS) $(DIR)

size: ALL
	$(SIZE) $(MAINS)

strip: ALL
	$(STRIP) $(MAINS)
