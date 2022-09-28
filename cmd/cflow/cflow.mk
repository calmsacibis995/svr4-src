#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cflow:cflow.mk	1.12.2.5"

ROOT=
OWN=bin
GRP=bin
CC=cc
CFLAGS= 	-O
LIBELF=
LDLIBS=
LINK_MODE=
LINT=		lint
LINTFLAGS=
CMDBASE=	..
INC=		$(ROOT)/usr/include
INCSYS=		$(ROOT)/usr/include
SGSBASE=	$(CMDBASE)/sgs
SGSINC=		$(SGSBASE)/inc
INS=		$(CMDBASE)/install/install.sh
CCSBIN=		$(ROOT)/usr/ccs/bin
CCSLIB=		$(ROOT)/usr/ccs/lib
USRBIN=		$(ROOT)/usr/bin
USRLIB=		$(ROOT)/usr/lib
INSDIR=		$(CCSLIB)
STRIP=		strip
SOURCES=	dag.c lpfx.c nmf.c flip.c
OBJECTS=	dag.o lpfx.o nmf.o flip.o
PRODUCTS=	dag lpfx nmf flip

LINTDIR=	../sgs/alint/common
INCLIST=	-I$(LINTDIR) -I$(INC)
CC_CMD=		$(CC) $(CFLAGS) $(INCLIST) 

all:	dag lpfx nmf flip

install:	all
	cp dag dag.bak
	cp lpfx lpfx.bak
	cp nmf nmf.bak
	cp flip flip.bak
	cp cflow.sh cflow
	$(STRIP) dag
	$(STRIP) lpfx
	$(STRIP) nmf
	$(STRIP) flip
	/bin/sh $(INS) -f $(INSDIR) dag
	/bin/sh $(INS) -f $(INSDIR) lpfx
	/bin/sh $(INS) -f $(INSDIR) nmf
	/bin/sh $(INS) -f $(INSDIR) flip
	/bin/sh $(INS) -f $(CCSBIN) cflow
	mv dag.bak dag
	mv lpfx.bak lpfx
	mv nmf.bak nmf
	mv flip.bak flip
	rm cflow

dag:	dag.c
	$(CC) $(CFLAGS) -I$(INC) -o dag dag.c $(LINK_MODE)

lpfx:	lpfx.c $(LINTDIR)/lnstuff.h
	if u3b2 || u3b5 || u3b15;\
	then\
		$(CC_CMD) -I$(SGSINC)/m32 -o lpfx lpfx.c $(LINK_MODE) -lmalloc;\
	elif i386;\
	then\
		$(CC_CMD) -I$(SGSINC)/i386 -o lpfx lpfx.c $(LINK_MODE) -lmalloc;\
	fi

nmf:	nmf.c
	$(CC) $(CFLAGS) -I$(INC) -o nmf nmf.c $(LINK_MODE)

flip:	flip.c
	$(CC) $(CFLAGS) -I$(INC) -o flip flip.c $(LINK_MODE)

clean:
	rm -f *.o a.out make.out core

clobber:	clean
	rm -f dag lpfx nmf flip

lintit:
	$(LINT) $(LINTFLAGS) -I$(INC) dag.c
	if u3b2 || u3b5 || u3b15;\
	then\
		$(LINT) $(LINTFLAGS) $(INCLIST) -I$(SGSINC)/m32 lpfx.c -lmalloc;\
	elif i386;\
	then\
		$(LINT) $(LINTFLAGS) $(INCLIST) -I$(SGSINC)/i386 lpfx.c -lmalloc;\
	fi
	$(LINT) $(LINTFLAGS) -I$(INC) nmf.c
	$(LINT) $(LINTFLAGS) -I$(INC) flip.c
