#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)rtld:rtld.mk	1.3"
#
# makefile for rtld
#
#

ROOT=
SGS=
OWN=bin
GRP=bin
CC=cc
CFLAGS=-O
LDLIBS=
LINT=lint
LINTFLAGS=
SGSBASE=../../../cmd/sgs
INS=$(SGSBASE)/sgs.install
INSDIR=
STRIP=strip
HFILES=
SOURCES=
OBJECTS=
PRODUCTS=
LIB=$(ROOT)/lib
USRLIB=$(ROOT)/usr/lib
DEFLIST=
MAKE=make
ENVPARMS=ROOT="$(ROOT)" SGS="$(SGS)" OWN="$(OWN)" GRP="$(GRP)" CC="$(CC)" CFLAGS="$(CFLAGS)" LDLIBS="$(LDLIBS)" LINT="$(LINT)" LINTFLAGS="$(LINTFLAGS)" INS="$(INS)"  STRIP="$(STRIP)" MAKE="$(MAKE)" LIB="$(LIB)" USRLIB="$(USRLIB)" SGSBASE="$(SGSBASE)"

#
all:	
	if u3b15 || u3b5 || u3b2 ;\
		then cd m32; $(MAKE) all $(ENVPARMS); \
		cd ..;\
	else if i386; \
		then cd i386; $(MAKE) all $(ENVPARMS); \
		cd ..;\
	else if sparc; \
		then cd sparc; $(MAKE) all $(ENVPARMS); \
		cd ..;\
		fi; \
		fi; \
	fi;

install:	all
	echo nothing to do

clean:
	#
	# remove intermediate files except object modules and temp library
	if u3b15 || u3b5 || u3b2 ;\
		then cd m32; $(MAKE) clean ; \
	else if i386; \
		then cd i386; $(MAKE) clean; \
	else if sparc; \
		then cd sparc; $(MAKE) clean; \
		fi; \
		fi; \
		fi;
		#
clobber:
	#
	# remove intermediate files
	if u3b15 || u3b5 || u3b2 ;\
		then cd m32; $(MAKE) clobber ; \
	else if i386; \
		then cd i386; $(MAKE) clobber; \
	else if sparc; \
		then cd sparc; $(MAKE) clobber; \
		fi; \
		fi; \
		fi;
		#

lintit:
	#
	if u3b2 || u3b5 || u3b15; \
		then cd m32 ; $(MAKE) lintit LINT=$(LINT) LINTFLAGS=$(LINTFLAGS) ROOT=$(ROOT) ;\
	else if i386; \
		then cd i386 ; $(MAKE) lintit LINT=$(LINT) LINTFLAGS=$(LINTFLAGS) ROOT=$(ROOT) ; \
	else if sparc; \
		then cd sparc ; $(MAKE) lintit LINT=$(LINT) LINTFLAGS=$(LINTFLAGS) ROOT=$(ROOT) ; \
	fi; \
	fi; \
	fi;

backup:	backup.sh
	/bin/sh backup.sh
