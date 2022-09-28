#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#	Portions Copyright (c) 1988, Sun Microsystems, Inc.
#	All Rights Reserved.

#ident	"@(#)split:split.mk	1.3.4.1"
#	Makefile for split

ROOT =

DIR = $(ROOT)/usr/bin

INC = $(ROOT)/usr/include

LDFLAGS = -s $(SHLIBS) 

CFLAGS = -O -I$(INC)

INS = install

MAKEFILE = split.mk

MAINS = split

STRIP = strip

OBJECTS =  split.o

SOURCES =  split.c

ALL:		$(MAINS)

split:	$(SOURCES)
	$(CC) $(CFLAGS)  -o split  split.c   $(LDFLAGS)

GLOBALINCS = $(INC)/stdio.h $(INC)/sys/types.h $(INC)/sys/statvfs.h

strip:
	$(STRIP) $(MAINS)
clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)


all : ALL

install: ALL
	$(INS) -f $(DIR)  -m 0555 -u bin -g bin $(MAINS)

