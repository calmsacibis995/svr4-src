#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)dircmp:./dircmp.mk	1.2.1.1"


#	Makefile for dircmp

ROOT =

DIR = $(ROOT)/usr/bin

INS = install

MAKEFILE = dircmp.mk

MAINS = dircmp

STRIP = strip

OBJECTS =  dircmp

SOURCES =  dircmp.sh

ALL:		$(MAINS)

dircmp:	 
	cp dircmp.sh dircmp

clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) $(MAINS)


all : ALL

install: ALL
	$(INS) -f $(DIR)  -m 0555 -u bin -g bin $(MAINS)

