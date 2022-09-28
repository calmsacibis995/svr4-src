#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)lxprof:bblk/bblk.mk	1.10.1.11"

PROF_SAVE	=
XPROF_INCS	=
SRCBASE		= common

ROOT		=
SGS		= m32
OWN		= bin
GRP		= bin
CC		= cc
CFLAGS		= -O
LDLIBS		= 
LINT		= lint
LINTFLAGS 	=
LINK_MODE	=

BIN		= $(ROOT)/bin
LIB		= $(ROOT)/lib
USRBIN		= $(ROOT)/usr/bin
USRLIB		= $(ROOT)/usr/lib
CCSBIN		= $(ROOT)/usr/ccs/bin
CCSLIB		= $(ROOT)/usr/ccs/lib

SGSBASE		= ../..
CMDBASE		= ../../..
LPBASE		= $(SGSBASE)/lprof
PLBBASE		= $(LPBASE)/libprof
INS		= $(SGSBASE)/sgs.install
INSDIR		= $(CCSBIN)
STRIP		= strip
HFILES		= comdef.h comext.h macdef.h
INCDIRS		= \
		-I . \
		-I $(SGSBASE)/inc/$(MACH) \
		-I $(SGSBASE)/inc/common \
		-I $(LPBASE)/hdr \
		$(XPROF_INCS)
SOURCES		= parse.c comglb.c
MACHSOURCES	= macglb.c mac.c
OBJECTS		= comglb.o macglb.o mac.o parse.o
PRODUCTS	= ../basicblk

LIBELF		= $(SGSBASE)/libelf/$(MACH)/libelf.a
PROFLIBD	= $(PLBBASE)
LIBSYMINT	= -L$(PROFLIBD) -lsymint
LDFLAGS		=
LIBS		= $(LIBELF)

ENVPARMS	= \
	ROOT="$(ROOT)" SGS="$(SGS)" OWN="$(OWN)" GRP="$(GRP)" \
	CC="$(CC)" CFLAGS="$(CFLAGS)" LDLIBS="$(LDLIBS)" \
	LINT="$(LINT)" \ LINTFLAGS="$(LINTFLAGS)" INS="$(INS)" \
	INSDIR="$(INSDIR)" STRIP="$(STRIP)" LIBELF="$(LIBELF)" \
	MACH="$(MACH)" MACHINC="$(MACHINC)" \
	CCSBIN="$(CCSBIN)" CCSLIB="$(CCSLIB)" \
	LINK_MODE="$(LINK_MODE)" \
	PROF_SAVE="$(PROF_SAVE)" LIBS="$(LIBS)" 

all: $(PRODUCTS)

$(PRODUCTS): $(OBJECTS)
	$(CC) -o $(PRODUCTS) $(CFLAGS) $(OBJECTS) \
	$(LDFLAGS) $(LIBS) $(LDLIBS) $(LINK_MODE)

mac.o: $(MACH)/mac.c
	$(CC) $(CFLAGS) -c $(INCDIRS) $(MACH)/mac.c

macglb.o: $(MACH)/macglb.c
	$(CC) $(CFLAGS) -c $(INCDIRS) $(MACH)/macglb.c

parse.o: $(HFILES) parse.c
	$(CC) $(CFLAGS) $(INCDIRS) -c parse.c -DMACH_IS_$(MACH)

install: all
	cp $(PRODUCTS) basicblk.bak
	$(STRIP) $(PRODUCTS)
	/bin/sh $(INS) 755 $(OWN) $(GRP) $(CCSLIB)/$(SGS)basicblk $(PRODUCTS)
	mv basicblk.bak $(PRODUCTS)

lintit:
	$(LINT) $(LINTFLAGS) $(INCDIRS) $(SOURCES)
	$(LINT) $(LINTFLAGS) -c $(INCDIRS) $(MACH)/$(MACHSOURCES)

clean:
	rm -f *.o

clobber: clean
	rm -f $(PRODUCTS)
