#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)shl:shl.mk	1.5.6.1"

ROOT=
INSDIR=$(ROOT)/usr/bin
INS = install
CFLAGS= -O
LDFLAGS= -s
LPDEST= laser
LEX=lex

OBJECTS= defs.o yacc.o newmain.o mpx.o layer.o misc.o stat.o signal.o \
	 system.o aux.o

all:		$(OBJECTS)
		$(CC) $(CFLAGS) $(LDFLAGS) -o shl $(OBJECTS) -ll $(PERFLIBS)

print:
		dupsendto $(LPDEST) shl.mk defs.h defs.c yacc.y lex.l \
				 newmain.c mpx.c layer.c misc.c stat.c  \
				 signal.c aux.c system.c
yacc.o	:	yacc.c lex.c

yacc.c	:	yacc.y

lex.c	:	lex.l
		$(LEX) lex.l
# fixing ECHO redefined - termios.h defines ECHO
		echo "# undef ECHO" >lex.c
		cat lex.yy.c >>lex.c
		rm lex.yy.c


aux.o defs.o newmain.o mpx.o layer.o misc.o stat.o signal.o system.o	: defs.h


install:	all
		$(INS) -o -f $(INSDIR) -m 4755 -u root -g bin shl

clobber:	clean
		rm -f shl
		rm -f $(INSDIR)/OLDshl

clean:		
		rm -f lex.c yacc.c *.o
