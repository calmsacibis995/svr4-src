#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)size:i386/makefile	1.3"
#	SIZE MAKEFILE
#
#
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

SGSBASE=../..
CMDBASE=..
MACHINC=$(SGSBASE)/inc/$(MACH)
COMINC=$(SGSBASE)/inc/common
COMDIR=../common
BIN=$(ROOT)/bin
CCSBIN=$(ROOT)/usr/ccs/bin
CCSLIB=$(ROOT)/usr/ccs/lib

INS=$(SGSBASE)/sgs.install
INSDIR=$(CCSBIN)

STRIP=strip

HFILES= $(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(COMINC)/scnhdr.h \
	$(MACHINC)/sgs.h $(COMDIR)/process.h $(COMDIR)/defs.h	

SOURCES=$(COMDIR)/main.c $(COMDIR)/process.c $(COMDIR)/fcns.c

OBJECTS=main.o process.o fcns.o

PRODUCTS=size

ARCH=AR32W
ARFORMAT=PORTAR
FLEX= -DFLEXNAMES
DEFLIST=-D$(ARCH) -D$(ARFORMAT) $(FLEX) -DUNIX

INC=$(ROOT)/usr/include

INCLIST=-I$(MACHINC) -I$(COMINC) -I$(COMDIR) 

CC_CMD=$(CC) $(CFLAGS) -c $(DEFLIST) $(INCLIST)

LIBELF=$(SGSBASE)/libelf/$(MACH)/libelf.a
#

all:	size

size:	$(OBJECTS) $(LIBELF)
	$(CC) -o size $(OBJECTS) $(LIBELF) $(LDLIBS) $(LINK_MODE)

lint:	$(OBJECTS) $(LIBELF)
	$(LINT) -o size $(OBJECTS) $(LIBELF) $(LDLIBS) 
#
main.o:	$(COMINC)/filehdr.h $(COMINC)/ldfcn.h $(MACHINC)/sgs.h \
	$(COMDIR)/defs.h $(COMDIR)/main.c
	$(CC_CMD) $(COMDIR)/main.c
#
process.o:	$(COMINC)/filehdr.h $(COMINC)/scnhdr.h $(COMINC)/ldfcn.h \
		$(COMDIR)/process.h $(COMDIR)/defs.h $(COMDIR)/process.c \
		$(MACHINC)/sgs.h
	$(CC_CMD) $(COMDIR)/process.c
#
fcns.o:	$(COMINC)/filehdr.h $(COMINC)/scnhdr.h $(COMINC)/ldfcn.h \
	$(MACHINC)/sgs.h $(COMDIR)/defs.h $(COMDIR)/fcns.c
	$(CC_CMD) $(COMDIR)/fcns.c
#
install: all
	cp size size.bak
	$(STRIP) size
	/bin/sh $(INS) 755 $(OWN) $(GRP) $(INSDIR)/$(SGS)size size
	mv size.bak size
#
lintit: $(SOURCES) $(HFILES)
	$(LINT) $(LINTFLAGS) $(DEFLIST) $(INCLIST) $(SOURCES)
#
clean:
	rm -f $(OBJECTS)
#
clobber: clean
	rm -f $(PRODUCTS)
