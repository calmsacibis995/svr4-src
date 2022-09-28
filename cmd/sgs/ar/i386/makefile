#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)ar:i386/makefile	1.1.1.3"
#***************************************************************************
#                                                                          *
#			UNIX Archive (ar) makefile			   *
#									   *
#***************************************************************************

ROOT=
SGS=i386
OWN=bin
GRP=bin
CC=cc
CFLAGS=-O
LINT=lint
LINTFLAGS=
LINK_MODE=

CCSBIN=$(ROOT)/usr/ccs/bin
CCSLIB=$(ROOT)/usr/ccs/lib

INC=$(ROOT)/usr/include
INCSYS=$(ROOT)/usr/include

BIN=$(ROOT)/bin
SGSBASE=../..
MACH=i386
MACHINC=$(SGSBASE)/inc/$(MACH)
COMINC=$(SGSBASE)/inc/common
LIBELF=$(SGSBASE)/libelf/$(MACH)/libelf.a
COMDIR=../common

INS=$(SGSBASE)/sgs.install
INSDIR=$(CCSBIN)

ARFORMAT= PORTAR

STRIP	= strip

HFILES = $(MACHINC)/sgs.h $(MACHINC)/paths.h $(COMINC)/libelf.h

SOURCES=$(COMDIR)/ar.c

OBJECTS=ar.o

PRODUCTS=ar

DEFLIST	= -D$(ARFORMAT) -DI386
INCLIST	= -I$(COMINC) -I$(MACHINC)
CC_CMD=$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST)

all:	$(PRODUCTS)


$(PRODUCTS):	$(OBJECTS) $(LIBELF)
	$(CC) $(OBJECTS) $(LINK_MODE) $(LIBELF) -o ar

ar.o:	$(HFILES) $(COMDIR)/ar.c
	$(CC_CMD) -c $(COMDIR)/ar.c

install: all	
	cp ar ar.bak
	$(STRIP) ar
	/bin/sh $(INS) 755 $(OWN) $(GRP) $(INSDIR)/$(SGS)ar ar
	mv ar.bak ar

lintit: $(SOURCES) $(HFILES)
	$(LINT) $(LINTFLAGS) $(DEFLIST) $(INCLIST) $(SOURCES)

clean:
	-rm -f $(OBJECTS)

clobber: clean
	-rm -f $(PRODUCTS)
