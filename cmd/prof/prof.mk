#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)prof:prof.mk	1.8.3.18"

PROF_SAVE	=

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

USRBIN		= $(ROOT)/usr/bin
USRLIB		= $(ROOT)/usr/lib
CCSBIN		= $(ROOT)/usr/ccs/bin
CCSLIB		= $(ROOT)/usr/ccs/lib

CMDBASE		= ..
SGSBASE		= ../sgs
LPBASE		= $(SGSBASE)/lprof
PLBBASE		= $(LPBASE)/libprof

MACHINC		= $(SGSBASE)/inc/$(MACH)
INS		= $(CMDBASE)/install/install.sh
INSDIR		= $(CCSBIN)
STRIP		= strip
HFILES		= \
		$(SGSBASE)/inc/$(MACH)/sgs.h \
		$(PLBBASE)/symint.h \
		$(PLBBASE)/symintHdr.h \
		$(PLBBASE)/debug.h
INCDIRS		= \
		-I $(SGSBASE)/inc/common \
		-I $(SGSBASE)/inc/$(MACH) \
		-I $(LPBASE)/hdr \
		-I $(PLBBASE)
SOURCES		= prof.c
OBJECTS		= prof.o
PRODUCTS	= prof

LIBELF		= -lelf
PROFLIBD	= $(PLBBASE)
LIBSYMINT	= -L$(PROFLIBD) -lsymint
LDFLAGS		= -s
LIBS		= $(LIBSYMINT) $(LIBELF)

ENVPARMS	= \
	ROOT="$(ROOT)" SGS="$(SGS)" OWN="$(OWN)" GRP="$(GRP)" \
	CC="$(CC)" CFLAGS="$(CFLAGS)" LDLIBS="$(LDLIBS)" \
	LINT="$(LINT)" \ LINTFLAGS="$(LINTFLAGS)" INS="$(INS)" \
	INSDIR="$(INSDIR)" STRIP="$(STRIP)" LIBELF="$(LIBELF)" \
	MACH="$(MACH)" MACHINC="$(MACHINC)" \
	CCSBIN="$(CCSBIN)" CCSLIB="$(CCSLIB)" \
	LINK_MODE="$(LINK_MODE)" \
	PROF_SAVE="$(PROF_SAVE)" LIBS="$(LIBS)" 


all:	
	if u3b2 || u3b5 || u3b15; \
	then \
	$(MAKE) -f prof.mk profmk MACH=m32; \
	elif i386; \
	then \
	$(MAKE) -f prof.mk profmk MACH=i386; \
	fi

profmk: libsymint $(PRODUCTS)

$(PRODUCTS): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LDFLAGS) \
	$(LDLIBS) $(LIBS) -o prof $(LINK_MODE)
	if test ! "$(PROF_SAVE)"; then rm -f $(PLBBASE)/libsymint.a; fi

prof.o:	$(HFILES) prof.c
	$(CC) $(CFLAGS) $(INCDIRS) -c $*.c

libsymint:
	cd $(PLBBASE); $(MAKE) -f libprof.mk $(ENVPARMS)

install: all
	cp $(PRODUCTS) prof.bak
	$(STRIP) $(PRODUCTS)
	sh $(INS) -f $(INSDIR) $(PRODUCTS)
	mv prof.bak $(PRODUCTS)

lintit:	$(SOURCES)
	$(LINT) $(LINTFLAGS) $(SOURCES)

clean:
	rm -f $(OBJECTS)

clobber: clean
	rm -f $(PRODUCTS)

