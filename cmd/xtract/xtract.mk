#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)xtract:xtract.mk	1.1"

#	Makefile for xtract

ROOT =

DIR = $(ROOT)/usr/bin

INC = $(ROOT)/usr/include

INS = install

LDFLAGS = -s -lgen $(LDLIBS)

CFLAGS = -O -I$(INC)

STRIP = strip

SIZE = size

MAKEFILE = xtract.mk

MAINS = xtract

OBJECTS =  xtract.o

SOURCES =  xtract.c

ALL:		$(MAINS)

xtract:	$(OBJECTS)
	$(CC) $(CFLAGS) -o $(MAINS) $(OBJECTS) $(LDFLAGS)

xtract.o:	 $(INC)/stdio.h \
		 $(INC)/signal.h \
		 $(INC)/sys/signal.h 

clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

all : ALL

install: ALL
	$(INS) -f $(DIR) -u root -m 0755 -g sys $(MAINS)

size: ALL
	$(SIZE) $(MAINS)

strip: ALL
	$(STRIP) $(MAINS)
