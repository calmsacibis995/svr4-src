#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)strip:i386/makefile	1.3"
#***************************************************************************
#                                                                          *
#			strip makefile					   *
#									   *
#***************************************************************************

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
CCSLIB=$(ROOT)/usr/ccs/lib
INC=$(ROOT)/usr/include
INCSYS=$(ROOT)/usr/include
LIB=$(ROOT)/lib
SGSBASE=../..
MACH=i386
MACHINC=$(SGSBASE)/inc/$(MACH)
COMINC=$(SGSBASE)/inc/common
LIBELF=$(SGSBASE)/libelf/$(MACH)/libelf.a
COMDIR=../common

BIN=$(ROOT)/bin
INS=$(SGSBASE)/sgs.install

INSDIR=$(CCSBIN)

ARFORMAT= PORTAR

STRIP	= strip

HFILES = $(MACHINC)/sgs.h $(MACHINC)/paths.h $(COMINC)/libelf.h

SOURCES=$(COMDIR)/main.c

OBJECTS=main.o

PRODUCTS=strip

DEFLIST	= -D$(ARFORMAT) -DI386

INCLIST	= -I$(COMINC) -I$(MACHINC) 

CC_CMD=$(CC) $(CFLAGS) -c $(DEFLIST) $(INCLIST)

all:	$(PRODUCTS)	

$(PRODUCTS):	$(OBJECTS) $(LIBELF)
	$(CC) -o strip $(OBJECTS) $(LINK_MODE) $(LIBELF)


main.o:		$(HFILES) $(COMDIR)/main.c
	$(CC_CMD) $(COMDIR)/main.c

install: all	
	cp strip strip.bak
	$(STRIP) strip
	/bin/sh $(INS) 755 $(OWN) $(GRP) $(INSDIR)/$(SGS)strip strip
	mv strip.bak strip

lintit: $(SOURCES) $(HFILES)
	$(LINT) $(LINTFLAGS) $(DEFLIST) $(INCLIST) $(SOURCES)

clean:
	-rm -f $(OBJECTS)

clobber: clean
	-rm -f $(PRODUCTS)
