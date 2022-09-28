#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)dis:i386/makefile	1.3"
#
#	DIS MAKEFILE
#
#
ROOT=
SGS=i386
OWN=bin
GRP=bin
CC=cc
CFLAGS=-O 
LDLIB=
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

STRIP=strip

HFILES= $(COMDIR)/structs.h  dis.h $(MACHINC)/paths.h $(MACHINC)/sgs.h $(COMINC)/libelf.h

SOURCES=$(COMDIR)/main.c $(COMDIR)/utls.c bits.c tables.c \
	$(COMDIR)/extn.c $(COMDIR)/lists.c $(COMDIR)/debug.c

OBJECTS=extn.o main.o utls.o lists.o bits.o tables.o debug.o

PRODUCTS=dis

ARCH=AR32WR
ARFORMAT=PORTAR
I386=I386
DBO=RBO

DEFLIST=-D$(ARCH) -D$(I386) -D$(DBO) -D$(ARFORMAT) 

INCLIST=-I$(MACHINC) -I$(COMINC) -I. -I$(COMDIR) 

CC_CMD=$(CC) $(CFLAGS) -c $(DEFLIST) $(INCLIST)

all:	$(PRODUCTS)

$(PRODUCTS):	$(OBJECTS) $(LIBELF)
	$(CC) -o dis $(OBJECTS) $(LINK_MODE) $(LIBELF) 

 
extn.o:		dis.h $(COMDIR)/structs.h $(COMDIR)/extn.c
	$(CC_CMD) $(COMDIR)/extn.c

main.o:		$(MACHINC)/paths.h $(MACHINC)/sgs.h \
		$(COMDIR)/structs.h $(COMDIR)/main.c
	$(CC_CMD) $(COMDIR)/main.c

utls.o:		dis.h $(MACHINC)/sgs.h $(COMDIR)/structs.h $(COMDIR)/utls.c
	$(CC_CMD) $(COMDIR)/utls.c

lists.o:	dis.h $(COMDIR)/structs.h $(MACHINC)/paths.h \
		$(MACHINC)/sgs.h $(COMDIR)/lists.c
	$(CC_CMD) $(COMDIR)/lists.c

debug.o:	dis.h $(COMDIR)/structs.h $(MACHINC)/sgs.h \
		$(MACHINC)/sgs.h $(COMINC)/dwarf.h $(COMDIR)/debug.c
	$(CC_CMD) $(COMDIR)/debug.c
	
bits.o:		dis.h $(MACHINC)/sgs.h $(COMINC)/libelf.h bits.c
	$(CC_CMD) bits.c

tables.o:	dis.h tables.c
	$(CC_CMD) tables.c


install:	all
	cp dis dis.bak
	$(STRIP) dis
	sh $(INS) 755 $(OWN) $(GRP) $(INSDIR)/$(SGS)dis dis
	mv dis.bak dis


lintit:	$(SOURCES) $(HFILES)
	$(LINT) $(LINTFLAGS)  $(DEFLIST) $(INCLIST) $(SOURCES)

clean:
	rm -rf $(OBJECTS)

clobber: clean
	rm -f $(PRODUCTS)
