#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)m4:i386/makefile	1.3"

ROOT=
SGS=i386
OWN=bin
GRP=bin
CC=cc
CFLAGS=-O
LDLIBS=
LINT=lint
LINTFLAGS=
LINK_MODE=

CCSBIN=$(ROOT)/usr/ccs/bin
SGSBASE=../..
COMDIR=../common

INS= $(SGSBASE)/sgs.install
INSDIR= $(CCSBIN)

STRIP=strip
YACC=yacc
YACCLIB=-ly

HFILES=$(COMDIR)/m4.h
SOURCES=$(COMDIR)/m4.c $(COMDIR)/m4ext.c $(COMDIR)/m4macs.c $(COMDIR/m4y.y
OBJECTS=m4.o m4ext.o m4macs.o m4y.o
PRODUCTS=m4

all:	$(PRODUCTS)


$(PRODUCTS):	$(OBJECTS)	
	$(CC) $(OBJECTS) $(YACCLIB) $(LDLIBS) $(LINK_MODE) -o m4

m4.o:	$(COMDIR)/m4.c $(COMDIR)/m4.h
	$(CC) $(CFLAGS) -c $(COMDIR)/m4.c

m4ext.o:	$(COMDIR)/m4ext.c $(COMDIR)/m4.h
	$(CC) $(CFLAGS) -c $(COMDIR)/m4ext.c

m4macs.o:	$(COMDIR)/m4macs.c $(COMDIR)/m4.h
	$(CC) $(CFLAGS) -c $(COMDIR)/m4macs.c

m4y.o:	$(COMDIR)/m4y.y
	$(YACC) $(COMDIR)/m4y.y
	$(CC) $(CFLAGS) -c y.tab.c
	mv y.tab.o m4y.o

install:	all
	cp m4 m4.bak
	$(STRIP) m4
	/bin/sh $(INS) 755 $(OWN) $(GRP) $(INSDIR)/$(SGS)m4 m4
	mv m4.bak m4

clean:
	-rm -f $(OBJECTS)
	
clobber:	clean
	-rm -f y.tab.c
	-rm -f $(PRODUCTS)	
