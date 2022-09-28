#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sgs-cmd:i386/makefile	1.1.1.7"

ROOT=
SGS=i386
OWN=bin
GRP=bin
CC=cc
CFLAGS=-O
LDLIBS =
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

HFILES=$(MACHINC)/paths.h $(MACHINC)/sgs.h

SOURCES=$(COMDIR)/cc.c

OBJECTS=cc.o

PRODUCTS=cc 

ARCH=AR32WR
DEFLIST=-D$(ARCH) -Di386

INCLIST=-I$(MACHINC) -I$(INC) -I$(INCSYS) -I$(COMINC)

CC_CMD=$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST)

all:	$(PRODUCTS)
	
$(PRODUCTS): 	$(OBJECTS)
	$(CC) $(OBJECTS) $(LINK_MODE) -o cc

cc.o: 	$(HFILES) $(COMDIR)/cc.c
	$(CC_CMD) -c $(COMDIR)/cc.c 

install:$(INSDIR)/$(SGS)cc 

$(INSDIR)/$(SGS)cc:cc
	cp cc cc.bak
	$(STRIP) cc
	/bin/sh $(INS) 755 $(OWN) $(GRP) $(INSDIR)/$(SGS)cc cc
	mv cc.bak cc

lintit:	$(HFILES) $(SOURCES)
	$(LINT) $(LINTFLAGS) $(DEFLIST) $(INCLIST) $(SOURCES)

clean:
	rm -f $(OBJECTS)

clobber: clean
	rm -f $(PRODUCTS)
