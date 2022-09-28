#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)ldd:ldd.mk	1.4"

# makefile for ldd (List Dynamic Dependencies)

ROOT=
SGS=
OWN=bin
GRP=bin
CC=cc
CFLAGS=-O
LDLIBS=
LINT=lint
LINTFLAGS=
STRIP=strip
HFILES=
OBJECTS=
CMDBASE=../..
CCSBIN=$(ROOT)/usr/ccs/bin
INS=$(CMDBASE)/install/install.sh
INSDIR=$(CCSBIN)
LINK_MODE=
MAKE=make
ENVPARMS=ROOT="$(ROOT)" SGS="$(SGS)" OWN="$(OWN)" GRP="$(GRP)" CC="$(CC)" \
	CFLAGS="$(CFLAGS)" LINT="$(LINT)" LINTFLAGS="$(LINTFLAGS)" \
	INS="$(INS)" INSDIR="$(INSDIR)" STRIP="$(STRIP)" LINK_MODE="$(LINK_MODE)"


all:
	if u3b2 || u3b5 || u3b15;\
	then \
		cd m32; \
		$(MAKE) all $(ENVPARMS) ;\
	elif i386; \
	then \
		cd i386; \
		$(MAKE) all $(ENVPARMS) ;\
	fi;

install:
	if u3b2 || u3b5 || u3b15;\
	then \
		cd m32; \
		$(MAKE) install $(ENVPARMS) ;\
	elif i386; \
	then \
		cd i386; \
		$(MAKE) install $(ENVPARMS) ;\
	fi;


lintit:
	if u3b2 || u3b5 || u3b15;\
	then \
		cd m32; \
		$(MAKE) lintit $(ENVPARMS) ;\
	elif i386; \
	then \
		cd i386; \
		$(MAKE) lintit $(ENVPARMS) ;\
	fi;

clean:
	if u3b2 || u3b5 || u3b15;\
	then \
		cd m32; \
		$(MAKE) clean ;\
	elif i386; \
	then \
		cd i386; \
		$(MAKE) clean ;\
	fi;

clobber:
	if u3b2 || u3b5 || u3b15;\
	then \
		cd m32; \
		$(MAKE) clobber ;\
	elif i386; \
	then \
		cd i386; \
		$(MAKE) clobber ;\
	fi;
