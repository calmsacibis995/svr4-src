#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libl:libl.mk	1.8.1.2"
ROOT=
SGS=
OWN=bin
GRP=bin
CC=cc
CFLAGS= -O
LDLIBS=
LINT=lint
LINTFLAGS=
SGSBASE=../../cmd/sgs
INC=$(ROOT)/usr/include
INCSYS=$(ROOT)/usr/include
INS=$(SGSBASE)/sgs.install
LIB=$(ROOT)/lib
USRLIB=$(ROOT)/usr/lib
CCSBIN=$(ROOT)/usr/ccs/bin
CCSLIB=$(ROOT)/usr/ccs/lib
LORDER=lorder
AR=ar
ARFLAGS=rv
SRCLIB=./lib
SOURCES=$(SRCLIB)/allprint.c $(SRCLIB)/main.c $(SRCLIB)/reject.c $(SRCLIB)/yyless.c $(SRCLIB)/yywrap.c
OBJECTS=allprint.o main.o reject.o yyless.o yywrap.o

all:     $(CCSLIB)/libl.a

$(CCSLIB)/libl.a: $(OBJECTS)
	$(AR) $(ARFLAGS) tmplib.a `$(LORDER) *.o | tsort`;

install:  ncform nrform all
	/bin/sh $(INS) 644 $(OWN) $(GRP) $(CCSLIB)/libl.a tmplib.a;

allprint.o:	$(SRCLIB)/allprint.c
	$(CC) -c $(CFLAGS) $(SRCLIB)/allprint.c
main.o:		$(SRCLIB)/main.c
	$(CC) -c $(CFLAGS) $(SRCLIB)/main.c
reject.o:	$(SRCLIB)/reject.c
	$(CC) -c $(CFLAGS) $(SRCLIB)/reject.c
yyless.o:	$(SRCLIB)/yyless.c
	$(CC) -c $(CFLAGS) $(SRCLIB)/yyless.c
yywrap.o:	$(SRCLIB)/yywrap.c
	$(CC) -c $(CFLAGS) $(SRCLIB)/yywrap.c

ncform:	$(SRCLIB)/ncform
	if [ ! -d $(CCSLIB)/lex ];\
		then mkdir $(CCSLIB)/lex;\
	fi
	/bin/sh $(INS) 644 $(OWN) $(GRP) $(CCSLIB)/lex/ncform $(SRCLIB)/ncform;

nrform:	$(SRCLIB)/nrform
	/bin/sh $(INS) 644 $(OWN) $(GRP) $(CCSLIB)/lex/nrform $(SRCLIB)/nrform;

clean:
	-rm -f $(OBJECTS)

clobber:clean
	-rm -f tmplib.a 

lintit:	$(SOURCES)
	$(LINT) $(LINTFLAGS) $(SOURCES)
