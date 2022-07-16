#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)nametoaddr:npack/npack.mk	1.1.3.2"
#	Makefile for npack.so

ROOT =

DIR = $(ROOT)/usr/lib

INC = $(ROOT)/usr/include

LDFLAGS = -s

CFLAGS = -O -I$(INC)

STRIP = strip

SIZE = size

MAKEFILE = npack.mk

MAINS = npack.so

OBJECTS =  npack.o

SOURCES =  npack.c

ALL:		$(MAINS)

npack.so:	npack.o
	$(CC) $(CFLAGS) -dy -G -ztext -o npack.so npack.o $(LDFLAGS)


npack.o:   $(INC)/stdio.h $(INC)/tiuser.h $(INC)/netdir.h \
	   $(INC)/netconfig.h $(INC)/ctype.h
	$(CC) $(CFLAGS) -c npack.c $(LDFLAGS) -Kpic

clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

all : ALL

install: ALL
	install -f $(DIR) npack.so

size: ALL
	$(SIZE) $(MAINS)

strip: ALL
	$(STRIP) $(MAINS)
