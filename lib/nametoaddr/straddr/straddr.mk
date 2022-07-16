#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)nametoaddr:straddr/straddr.mk	1.1.3.2"

#	Makefile for straddr.so

ROOT =

DIR = $(ROOT)/usr/lib

INC = $(ROOT)/usr/include

LDFLAGS = -s

CFLAGS = -O -I$(INC)

STRIP = strip

SIZE = size

MAKEFILE = straddr.mk

MAINS = straddr.so

OBJECTS =  straddr.o

SOURCES =  straddr.c

ALL:		$(MAINS)

straddr.so:	straddr.o
	$(CC) $(CFLAGS) -dy -G -ztext -o straddr.so straddr.o $(LDFLAGS)


straddr.o:   $(INC)/stdio.h $(INC)/tiuser.h $(INC)/netdir.h \
	   $(INC)/netconfig.h $(INC)/ctype.h
	$(CC) $(CFLAGS) -c straddr.c $(LDFLAGS) -Kpic

clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)

all : ALL

install: ALL
	install -f $(DIR) straddr.so

size: ALL
	$(SIZE) $(MAINS)

strip: ALL
	$(STRIP) $(MAINS)
