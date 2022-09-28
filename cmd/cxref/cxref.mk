#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cxref:cxref.mk	1.17.2.1"

ROOT=
OWN=bin
GRP=bin
CC=cc
CFLAGS=-O
LIBELF=
LDLIBS=
LINK_MODE=
LINT=lint
LINTFLAGS=
SGSBASE=../sgs
INC=$(ROOT)/usr/include
INCSYS=$(ROOT)/usr/include

INCPATH=$(SGSBASE)/inc
BIN=$(ROOT)/bin
CCSBIN=$(ROOT)/usr/ccs/bin
CCSLIB=$(ROOT)/usr/ccs/lib
USRBIN=$(ROOT)/usr/bin
USRLIB=$(ROOT)/usr/lib
CMDBASE=..
INS=$(CMDBASE)/install/install.sh
INSDIR=$(CCSBIN)
STRIP=strip
SOURCES=cxref.c st.c
OBJECTS=cxref.o st.o
PRODUCTS=xref

all:		$(PRODUCTS)

xref:		$(OBJECTS)
		$(CC) $(OBJECTS) $(LINK_MODE) $(LDLIBS) -o xref

cxref.o:	cxref.c cxref.h
		if u3b2 || u3b5 || u3b15;\
		then\
			$(CC) $(CFLAGS) -I$(INCPATH)/m32 -I$(INC) -c cxref.c;\
		elif i386;\
		then\
			$(CC) $(CFLAGS) -I$(INCPATH)/i386 -I$(INC) -c cxref.c;\
		fi

st.o:		st.c
		$(CC) $(CFLAGS) -I$(INC) -c st.c

install:	all
		cp xref xref.bak
		$(STRIP) xref
		cp cxref.sh cxref
		/bin/sh $(INS) -f $(CCSLIB) xref
		/bin/sh $(INS) -f $(INSDIR) cxref
		rm -f cxref
		mv xref.bak xref

lintit:		$(SOURCES)
		if u3b2 || u3b5 || u3b15;\
		then\
			$(LINT) $(LINTFLAGS) -I$(INCPATH)/m32 -I$(INC) $(SOURCES);\
		elif i386;\
		then\
			$(LINT) $(LINTFLAGS) -I$(INCPATH)/i386 -I$(INC) $(SOURCES);\
		fi

clean:
		-rm -f $(OBJECTS)

clobber:	clean
		-rm -f $(PRODUCTS)
