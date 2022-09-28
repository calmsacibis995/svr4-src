#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)yacc:i386/makefile	1.10"

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

USRBIN=$(ROOT)/usr/bin
USRLIB=$(ROOT)/usr/lib
SGSBASE=../..
MACH=i386
INS=$(SGSBASE)/sgs.install
MACHINC=$(SGSBASE)/inc/$(MACH)
INSDIR=$(CCSBIN)
COMDIR=../common

STRIP=strip

SOURCES=$(COMDIR)/y1.c $(COMDIR)/y2.c $(COMDIR)/y3.c $(COMDIR)/y4.c
OBJECTS=y1.o y2.o y3.o y4.o
PRODUCTS=yacc

all:	$(PRODUCTS)

yacc:	$(OBJECTS)
	$(CC) $(OBJECTS) $(LINK_MODE) $(LDLIBS) -o yacc 

$(OBJECTS):	$(COMDIR)/dextern 
		$(CC) -c $(CFLAGS) -DPARSER=\"$(CCSLIB)/yaccpar\" -I$(INC) -I$(MACHINC) $(SOURCES)

install:	all
		cp yacc yacc.bak
		$(STRIP) yacc
		/bin/sh $(INS) 755 $(OWN) $(GRP) $(INSDIR)/$(SGS)yacc yacc
		mv yacc.bak yacc
		/bin/sh $(INS) 644 $(OWN) $(GRP) $(CCSLIB)/yaccpar $(COMDIR)/yaccpar

lintit:	$(SOURCES)	
	$(LINT) $(LINTFLAGS) -I$(INC) -I$(MACHINC) $(SOURCES)

clean:
		-rm -f $(OBJECTS)

clobber:	clean
		-rm -f $(PRODUCTS)
