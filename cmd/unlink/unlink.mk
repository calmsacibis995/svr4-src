#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)unlink:unlink.mk	1.2.2.1"

#	Makefile for unlink

ROOT =

DIR = $(ROOT)/usr/sbin

SYMLINK = :

INC = $(ROOT)/usr/include

LDFLAGS = -s $(SHLIBS)

CFLAGS = -O -I$(INC)

INS = install

MAKEFILE = unlink.mk

MAINS = unlink

OBJECTS =  unlink.o

SOURCES =  unlink.c

ALL:		$(MAINS)

unlink:	$(SOURCES)
	$(CC) $(CFLAGS)  -o unlink  unlink.c   $(LDFLAGS)

clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)


all : ALL

install: ALL
	-rm -f $(ROOT)/etc/unlink
	$(INS) -f $(DIR) -m 0500 -u root -g bin $(MAINS)
	-$(SYMLINK) /usr/sbin/unlink $(ROOT)/etc/unlink

