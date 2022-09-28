#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)tsort:i386/makefile	1.1.1.2"

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

INS=$(SGSBASE)/sgs.install
INSDIR=$(CCSBIN)

STRIP=strip

HFILES=$(COMDIR)/errmsg.h
SOURCES=$(COMDIR)/Err.c $(COMDIR)/erraction.c $(COMDIR)/errafter.c \
	$(COMDIR)/errbefore.c $(COMDIR)/errexit.c $(COMDIR)/errprefix.c \
	$(COMDIR)/errsource.c $(COMDIR)/errstrtok.c $(COMDIR)/errtag.c \
	$(COMDIR)/errtext.c $(COMDIR)/errtofix.c $(COMDIR)/errusage.c \
	$(COMDIR)/tsort.c $(COMDIR)/zfopen.c $(COMDIR)/zmalloc.c

OBJECTS=Err.o erraction.o errafter.o errbefore.o errexit.o errprefix.o \
	errsource.o errstrtok.o errtag.o errtext.o errtofix.o errusage.o \
	tsort.o zfopen.o zmalloc.o

PRODUCTS=tsort

all:	$(PRODUCTS)

$(PRODUCTS):	$(OBJECTS)
	$(CC) $(OBJECTS) $(LDLIBS) $(LINK_MODE) -o tsort

Err.o:	$(COMDIR)/errmsg.h $(COMDIR)/Err.c
	$(CC) $(CFLAGS) -c $(COMDIR)/Err.c

erraction.o:	$(COMDIR)/errmsg.h $(COMDIR)/erraction.c
	$(CC) $(CFLAGS) -c $(COMDIR)/erraction.c

errafter.o:	$(COMDIR)/errmsg.h $(COMDIR)/errafter.c
	$(CC) $(CFLAGS) -c $(COMDIR)/errafter.c

errbefore.o:	$(COMDIR)/errmsg.h $(COMDIR)/errbefore.c
	$(CC) $(CFLAGS) -c $(COMDIR)/errbefore.c

errexit.o:	$(COMDIR)/errmsg.h $(COMDIR)/errexit.c
	$(CC) $(CFLAGS) -c $(COMDIR)/errexit.c

errprefix.o:	$(COMDIR)/errmsg.h $(COMDIR)/errprefix.c
	$(CC) $(CFLAGS) -c $(COMDIR)/errprefix.c

errsource.o:	$(COMDIR)/errmsg.h $(COMDIR)/errsource.c
	$(CC) $(CFLAGS) -c $(COMDIR)/errsource.c

errstrtok.o:	$(COMDIR)/errmsg.h $(COMDIR)/errstrtok.c
	$(CC) $(CFLAGS) -c $(COMDIR)/errstrtok.c

errtag.o:	$(COMDIR)/errmsg.h $(COMDIR)/errtag.c
	$(CC) $(CFLAGS) -c $(COMDIR)/errtag.c

errtext.o:	$(COMDIR)/errmsg.h $(COMDIR)/errtext.c
	$(CC) $(CFLAGS) -c $(COMDIR)/errtext.c

errtofix.o:	$(COMDIR)/errmsg.h $(COMDIR)/errtofix.c
	$(CC) $(CFLAGS) -c $(COMDIR)/errtofix.c

errusage.o:	$(COMDIR)/errmsg.h $(COMDIR)/errusage.c
	$(CC) $(CFLAGS) -c $(COMDIR)/errusage.c

tsort.o:	$(COMDIR)/errmsg.h $(COMDIR)/tsort.c
	$(CC) $(CFLAGS) -c $(COMDIR)/tsort.c

zfopen.o:	$(COMDIR)/errmsg.h $(COMDIR)/zfopen.c
	$(CC) $(CFLAGS) -c $(COMDIR)/zfopen.c

zmalloc.o:	$(COMDIR)/errmsg.h $(COMDIR)/zmalloc.c
	$(CC) $(CFLAGS) -c $(COMDIR)/zmalloc.c

install:	all
	cp tsort tsort.bak
	$(STRIP) tsort
	/bin/sh $(INS) 755 $(OWN) $(GRP) $(INSDIR)/$(SGS)tsort tsort
	mv tsort.bak tsort

lintit:	$(SOURCES)
	$(LINT) $(LINTFLAGS) $(SOURCES)

clean:	
	-rm -f $(OBJECTS)

clobber:	clean
	-rm -f $(PRODUCTS)
