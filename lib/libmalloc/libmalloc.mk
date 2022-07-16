#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libmalloc:libmalloc.mk	1.21.1.2"
#
# makefile for libmalloc
#

.SUFFIXES: .p
ROOT=
AR=ar
ARFLAGS=r
CC=cc
CFLAGS=-O
CP=cp
ENV=
INC= $(ROOT)/usr/include
CCSLIB=$(ROOT)/usr/ccs/lib
LIBP= $(CCSLIB)/libp
LINT=lint
LINTFLAGS=-u -m -lmalloc
MV=mv
SOURCES=malloc.c
OBJECTS=malloc.o
POBJECTS=malloc.p
RM=rm
TMPDIR=/usr/tmp
FRC=
OWN=bin
GRP=bin
SGSBASE=../../cmd/sgs

all: noprof prof

noprof: $(OBJECTS)
	$(RM) -rf objects
	mkdir objects
	$(CP) $(OBJECTS) objects
	cd objects; $(AR) $(ARFLAGS) libmalloc.a *

prof: $(POBJECTS)
	$(RM) -rf pobjects
	mkdir pobjects
	for i in $(POBJECTS);\
	do\
		$(CP) $$i pobjects/`basename $$i .p`.o;\
	done
	cd pobjects; $(AR) $(ARFLAGS) libmalloc.a *

malloc.o malloc.p: $(INC)/malloc.h mallint.h

.c.o:
	@echo $*.c:
	$(CC) $(CFLAGS) -c $*.c

.c.p:
	@echo $*.c:
	$(PROF)$(CC) -p $(CFLAGS) -c $*.c    && $(MV) $(*F).o $*.p

lintit: $(SOURCES)
	$(LINT) $(LINTFLAGS) $(SOURCES)

install: all
	if [ ! -d $(LIBP) ];\
	then\
		mkdir $(LIBP);\
	fi
	sh $(SGSBASE)/sgs.install 644 $(OWN) $(GRP) $(CCSLIB)/libmalloc.a objects/libmalloc.a
	sh $(SGSBASE)/sgs.install 644 $(OWN) $(GRP) $(LIBP)/libmalloc.a pobjects/libmalloc.a
	if pdp11; then \
		$(USRLIB)/libmalloc.a; \
		$(LIBP)/libmalloc.a; \
	fi

clean: 
	$(RM) -fr pobjects objects

clobber: clean
	$(RM) -f $(POBJECTS) $(OBJECTS)
