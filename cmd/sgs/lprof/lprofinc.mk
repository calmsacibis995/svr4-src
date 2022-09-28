#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)lprof:lprofinc.mk	1.3"

PROF_SAVE	=
SRCBASE		= common
NATIVE		= yes

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

MACHINC		= $(SGSBASE)/inc/$(MACH)
INS		= $(SGSBASE)/sgs.install
INSDIR		= $(CCSBIN)
STRIP		= strip
HFILES		= 
SOURCES		=
OBJECTS		=
PRODUCTS	=

LIBELF		= $(SGSBASE)/libelf/$(MACH)/libelf.a
PLBBASE		= $(SGSBASE)/lprof/libprof
PROFLIBD	= $(PLBBASE)
LIBPROF		= -L$(PROFLIBD) -lprof
LDFLAGS		= -s

ENVPARMS	= \
	ROOT="$(ROOT)" SGS="$(SGS)" OWN="$(OWN)" GRP="$(GRP)" \
	CC="$(CC)" CFLAGS="$(CFLAGS)" LDLIBS="$(LDLIBS)" \
	LINT="$(LINT)" LINTFLAGS="$(LINTFLAGS)" INS="$(INS)" \
	INSDIR="$(INSDIR)" STRIP="$(STRIP)" LIBELF="$(LIBELF)" \
	MACH="$(MACH)" MACHINC="$(MACHINC)" NATIVE="$(NATIVE)" \
	CCSBIN="$(CCSBIN)" CCSLIB="$(CCSLIB)" \
	LINK_MODE="$(LINK_MODE)" \
	PROF_SAVE="$(PROF_SAVE)" LIBPROF="$(LIBPROF)"

