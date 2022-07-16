#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libdl:libdl.mk	1.5"
#
# makefile for libdl
#
#

ROOT=
SGS=
OWN=bin
GRP=bin
CC=cc
CFLAGS=
LDLIBS=
LINT=lint
LINTFLAGS=
SGSBASE=../../cmd/sgs
INS=$(SGSBASE)/sgs.install
INSDIR=
STRIP=strip
HFILES=
SOURCES=
CCSLIB=$(ROOT)/usr/ccs/lib
OBJECTS= $(MACH)/dlclose.o $(MACH)/dlerror.o $(MACH)/dlopen.o $(MACH)/dlsym.o
PRODUCTS=
DEFLIST=
MAKE=make
ENVPARMS=ROOT="$(ROOT)" SGS="$(SGS)" OWN="$(OWN)" GRP="$(GRP)" CC="$(CC)" CFLAGS="$(CFLAGS)" LDLIBS="$(LDLIBS)" LINT="$(LINT)" LINTFLAGS="$(LINTFLAGS)" INS="$(INS)"  STRIP="$(STRIP)" MAKE="$(MAKE)" LIB="$(LIB)" USRLIB="$(USRLIB)" SGSBASE="$(SGSBASE)"

#
all:
	if u3b2 || u3b5 || u3b15; \
	then \
	$(MAKE) -f libdl.mk libdl.so MACH=m32; \
	elif i386; \
	then \
	$(MAKE) -f libdl.mk libdl.so MACH=i386; \
	fi

libdl.so:	$(OBJECTS)
	$(AR) -r libdl.so $(OBJECTS)

$(MACH)/dlclose.o:	$(MACH)/dlclose.s
	$(CC) -c $(CFLAGS) $(MODE) $(DEFLIST) $(INCLIST) $(MACH)/dlclose.s
	mv dlclose.o $(MACH)

$(MACH)/dlerror.o:	$(MACH)/dlerror.s
	$(CC) -c $(CFLAGS) $(MODE) $(DEFLIST) $(INCLIST) $(MACH)/dlerror.s
	mv dlerror.o $(MACH)

$(MACH)/dlopen.o:	$(MACH)/dlopen.s
	$(CC) -c $(CFLAGS) $(MODE) $(DEFLIST) $(INCLIST) $(MACH)/dlopen.s
	mv dlopen.o $(MACH)

$(MACH)/dlsym.o:	$(MACH)/dlsym.s
	$(CC) -c $(CFLAGS) $(MODE) $(DEFLIST) $(INCLIST) $(MACH)/dlsym.s
	mv dlsym.o $(MACH)

install:	all
	/bin/sh $(INS) 755 $(OWN) $(GRP) $(CCSLIB)/libdl.so libdl.so

clean:
	rm -f */*.o

clobber:	clean
	rm -f libdl.so
