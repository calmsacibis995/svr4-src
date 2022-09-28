#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)lex:i386/makefile	1.6"

ROOT=
CCSBIN=$(ROOT)/usr/ccs/bin
CCSLIB=$(ROOT)/usr/ccs/lib
SGS=i386
OWN=bin
GRP=bin
CC=cc
CFLAGS=-O
LIBELF=
LDLIBS=
LINK_MODE=
LINT=lint
LINTFLAGS=
INC=$(ROOT)/usr/include
INCSYS=$(ROOT)/usr/include

LEXDIR=$(CCSLIB)/lex
USRBIN=$(ROOT)/usr/bin
SGSBASE=../..
MACH=i386
INS=$(SGSBASE)/sgs.install
MACHINC=$(SGSBASE)/inc/$(MACH)
INSDIR=$(CCSBIN)
COMDIR=../common

STRIP=strip
YACC=yacc

SOURCES=$(COMDIR)/main.c $(COMDIR)/sub1.c $(COMDIR)/sub2.c $(COMDIR)/header.c $(COMDIR)/parser.y
OBJECTS=main.o sub1.o sub2.o header.o y.tab.o
PRODUCTS=lex

all:	$(PRODUCTS)

lex:	$(OBJECTS)
	$(CC) $(OBJECTS) $(LINK_MODE) $(LDLIBS) -ly -o lex 

main.o:	$(COMDIR)/main.c $(COMDIR)/ldefs.c $(COMDIR)/once.c
	$(CC) $(CFLAGS) -c -DCNAME=\"$(LEXDIR)/ncform\" \
	-DRATNAME=\"$(LEXDIR)/nrform\" -I$(INC) -I$(MACHINC) $(COMDIR)/main.c

sub1.o:	$(COMDIR)/sub1.c $(COMDIR)/ldefs.c
	$(CC) $(CFLAGS) -c -I$(INC) -I$(MACHINC) $(COMDIR)/sub1.c

sub2.o:	$(COMDIR)/sub2.c $(COMDIR)/ldefs.c
	$(CC) $(CFLAGS) -c -I$(INC) -I$(MACHINC) $(COMDIR)/sub2.c

header.o:	$(COMDIR)/header.c $(COMDIR)/ldefs.c
	$(CC) $(CFLAGS) -c -I$(INC) -I$(MACHINC) $(COMDIR)/header.c

y.tab.o:	$(COMDIR)/parser.y
		$(YACC) $(COMDIR)/parser.y
		$(CC) $(CFLAGS) -c -I$(INC) -I$(MACHINC) -I$(COMDIR) y.tab.c

install:	all
		cp lex lex.bak
		$(STRIP) lex
		/bin/sh $(INS) 755 $(OWN) $(GRP) $(INSDIR)/$(SGS)lex lex
		mv lex.bak lex

lintit:	$(SOURCES)	
	$(LINT) $(LINTFLAGS) -I$(INC) -I$(MACHINC) $(SOURCES)

clean:
		-rm -f $(OBJECTS)

clobber:	clean
		-rm -f y.tab.c
		-rm -f $(PRODUCTS)
