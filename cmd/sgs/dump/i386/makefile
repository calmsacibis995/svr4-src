#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)dump:i386/makefile	1.3"

#
#
#	DUMP MAKEFILE - I386 VERSION
#
#

ROOT=
SGS=i386
OWN=bin
GRP=bin
CC=cc
CFLAGS=-O
LINT=lint
LINTFLAGS=
LINK_MODE=

INC=$(ROOT)/usr/include
INCSYS=$(ROOT)/usr/include
LIB=$(ROOT)/lib
SGSBASE=../..
MACH=i386
MACHINC=$(SGSBASE)/inc/$(MACH)
COMINC=$(SGSBASE)/inc/common
LIBELF=$(SGSBASE)/libelf/$(MACH)/libelf.a
COMDIR=../common
LDLIBS=
CCSBIN=$(ROOT)/usr/ccs/bin
CCSLIB=$(ROOT)/usr/ccs/lib

BIN=$(ROOT)/bin
INS=$(SGSBASE)/sgs.install
INSDIR=$(CCSBIN)

STRIP=strip

HFILES=$(COMDIR)/dumpmap.h $(COMDIR)/dump.h \
       $(MACHINC)/sgs.h $(MACHINC)/paths.h \
       $(COMINC)/libelf.h $(COMINC)/dwarf.h

SOURCES=$(COMDIR)/dump.c $(COMDIR)/fcns.c $(COMDIR)/debug.c
OBJECTS=dump.o fcns.o debug.o
PRODUCTS=dump
ARFORMAT=PORTAR

ARCH=
DEFLIST=-D$(ARFORMAT) -DELF_TARGET_ALL
INCLIST=-I$(MACHINC) -I$(COMINC)
CC_CMD=$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST)



all:	dump

dump: 	$(OBJECTS) $(LIBELF) $(LDLIBS)
	$(CC) $(OBJECTS) $(LIBELF) $(LDLIBS) $(LINK_MODE) -o dump

dump.o: 	$(HFILES) $(COMDIR)/dump.c
	$(CC_CMD) -c $(COMDIR)/dump.c 


fcns.o: $(HFILES) $(COMDIR)/fcns.c
	$(CC_CMD) -c $(COMDIR)/fcns.c

debug.o: $(HFILES) $(COMDIR)/debug.c
	$(CC_CMD) -c $(COMDIR)/debug.c

install:	$(INSDIR)/$(SGS)dump 

$(INSDIR)/$(SGS)dump:	dump
	cp dump dump.bak
	$(STRIP) dump
	/bin/sh $(INS) 755 $(OWN) $(GRP) $(INSDIR)/$(SGS)dump dump
	mv dump.bak dump

lintit:	$(HFILES) $(SOURCES)
	$(LINT) $(LINTFLAGS) $(DEFLIST) $(INCLIST) $(SOURCES)

clean:
	-rm -f $(OBJECTS)

shrink clobber: clean
	-rm -f $(PRODUCTS)

