#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)link:link.mk	1.2.2.1"

#	Makefile for link

ROOT =

DIR = $(ROOT)/usr/sbin

SYMLINK = :

INC = $(ROOT)/usr/include

LDFLAGS = -s $(LDLIBS)

CFLAGS = -O -I$(INC)

INS = install

MAKEFILE = link.mk

MAINS = link

OBJECTS =  link.o

SOURCES =  link.c

ALL:		$(MAINS)

link:	$(SOURCES)
	$(CC) $(CFLAGS)  -o link  link.c   $(LDFLAGS) $(SHLIBS)

clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)


all : ALL

install: ALL
	-rm -f $(ROOT)/etc/link
	$(INS) -f $(DIR) -m 0500 -u root -g bin $(MAINS)
	-$(SYMLINK) /usr/sbin/link $(ROOT)/etc/link

