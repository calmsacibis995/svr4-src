#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)cscope:cscope.mk	1.13"
SGS=
ROOT=
BASE= ..
PROGRAM = $(SGS)cscope
#
INS=$(BASE)/install/install.sh
USRBIN=$(ROOT)/usr/bin
CCSBIN=$(ROOT)/usr/ccs/bin
CCSLIB=$(ROOT)/usr/ccs/lib
INSDIR=$(CCSBIN)
LISTU3B=-I$(BASE)/sgs/inc/u3b
LISTM32=-I$(BASE)/sgs/inc/m32
LISTI386=-I$(BASE)/sgs/inc/i386

INC=$(ROOT)/usr/include
INCSYS=$(ROOT)/usr/include
COMINC=$(BASE)/sgs/inc/common
COMCS= common
#
CC=cc
STRIP = strip
INCLIST= -I$(COMCS) -I$(COMINC) -I$(INC) 
DEFLIST= -DCCS
LIBELF=
LINK_MODE=
#
LINT= lint
LFLAGS=
OWN= bin
GRP= bin
#
LIBS= -lcurses -ll -lgen
MORECFLAGS= 
CFLAGS= -O -c $(MORECFLAGS) 

OBJ = main.o dir.o crossref.o scanner.o lookup.o command.o display.o find.o \
	edit.o exec.o help.o history.o input.o mouse.o
LIBOBJ = alloc.o basename.o compath.o egrep.o getwd.o logdir.o mygetenv.o \
	  mypopen.o vpaccess.o vpfopen.o vpinit.o
OBJECTS = $(OBJ) $(LIBOBJ)

CFILES =  $(COMCS)/main.c $(COMCS)/dir.c $(COMCS)/crossref.c \
	scanner.c $(COMCS)/lookup.c $(COMCS)/command.c \
	$(COMCS)/display.c $(COMCS)/find.c $(COMCS)/edit.c \
	$(COMCS)/exec.c $(COMCS)/help.c $(COMCS)/history.c $(COMCS)/input.c \
	$(COMCS)/mouse.c $(COMCS)/alloc.c $(COMCS)/basename.c \
	$(COMCS)/compath.c egrep.c $(COMCS)/getwd.c $(COMCS)/logdir.c \
	$(COMCS)/mygetenv.c $(COMCS)/mypopen.c $(COMCS)/vpaccess.c \
	$(COMCS)/vpfopen.c $(COMCS)/vpinit.c

all:	$(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LINK_MODE) $(LIBS)

main.o:	$(COMCS)/constants.h $(COMCS)/version.h \
	$(COMCS)/main.c
	$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(COMCS)/main.c

dir.o:	$(COMCS)/constants.h $(COMCS)/vp.h \
	$(COMCS)/dir.c
	$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(COMCS)/dir.c
	
crossref.o:	$(COMCS)/constants.h \
	$(COMCS)/crossref.c
	$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(COMCS)/crossref.c

scanner.c:	$(COMCS)/constants.h $(COMCS)/scanner.l
	$(LEX) -n -t $(COMCS)/scanner.l >scanner.c
scanner.o:	scanner.c $(COMCS)/global.h
	$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) scanner.c

lookup.o:	$(COMCS)/constants.h \
	$(COMCS)/lookup.c
	$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(COMCS)/lookup.c

command.o:	$(COMCS)/constants.h \
	$(COMCS)/command.c
	$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(COMCS)/command.c

display.o:	$(COMCS)/constants.h $(COMCS)/version.h \
	$(COMCS)/display.c
	if (u3b); then \
		$(CC) $(CFLAGS) $(DEFLIST) $(LISTU3B) $(INCLIST) $(COMCS)/display.c; \
	else \
	    if (u3b2); then \
		$(CC) $(CFLAGS) $(DEFLIST) $(LISTM32) $(INCLIST) $(COMCS)/display.c; \
	else \
	    if (i386); then \
		$(CC) $(CFLAGS) $(DEFLIST) $(LISTI386) $(INCLIST) $(COMCS)/display.c; \
	    fi \
	    fi \
	fi
		
find.o:	$(COMCS)/constants.h \
	$(COMCS)/find.c
	$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(COMCS)/find.c

edit.o: $(COMCS)/constants.h \
	$(COMCS)/edit.c
	$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(COMCS)/edit.c

exec.o:	$(COMCS)/constants.h \
	$(COMCS)/exec.c
	$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(COMCS)/exec.c

help.o:	$(COMCS)/constants.h \
	$(COMCS)/help.c
	$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(COMCS)/help.c

history.o:	$(COMCS)/constants.h \
	$(COMCS)/history.c
	$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(COMCS)/history.c

input.o: $(COMCS)/constants.h \
	$(COMCS)/input.c
	$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(COMCS)/input.c

mouse.o: $(COMCS)/constants.h \
	$(COMCS)/mouse.c
	$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(COMCS)/mouse.c

alloc.o:	$(COMCS)/alloc.c
	$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(COMCS)/alloc.c

basename.o:	$(COMCS)/basename.c
	$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(COMCS)/basename.c

compath.o:	$(COMCS)/compath.c
	$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(COMCS)/compath.c

egrep.c:	$(COMCS)/egrep.y
	$(YACC) $(COMCS)/egrep.y
	mv y.tab.c egrep.c
egrep.o:	egrep.c
	$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) egrep.c

getwd.o:	$(COMCS)/getwd.c
	$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(COMCS)/getwd.c

logdir.o:	$(COMCS)/logdir.c
	$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(COMCS)/logdir.c

mygetenv.o:	$(COMCS)/mygetenv.c
	$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(COMCS)/mygetenv.c

mypopen.o:	$(COMCS)/mypopen.c
	$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(COMCS)/mypopen.c

vpaccess.o:	$(COMCS)/vp.h $(COMCS)/vpaccess.c
	$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(COMCS)/vpaccess.c

vpfopen.o:	$(COMCS)/vp.h $(COMCS)/vpfopen.c
	$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(COMCS)/vpfopen.c

vpinit.o:	$(COMCS)/vp.h $(COMCS)/vpinit.c
	$(CC) $(CFLAGS) $(DEFLIST) $(INCLIST) $(COMCS)/vpinit.c

install: all
	cp $(PROGRAM) $(PROGRAM).bak
	$(STRIP) $(PROGRAM)
	/bin/sh $(INS) -f $(INSDIR) $(PROGRAM)
	mv $(PROGRAM).bak $(PROGRAM)
clean:
	rm -f *.o *.out lex.yy.c y.tab.? scanner.c egrep.c

clobber: clean
	rm -f $(PROGRAM)

strip:	$(PROGRAM)
	-$(STRIP) $(PROGRAM)

lintit:	$(CFILES)
	if (u3b); then \
		$(LINT) $(LFLAGS) $(LISTU3B) $(INCLIST) $(DEFLIST) $(ODEFLIST) $(LIBS) $(CFILES) ; \
	else \
	    if (u3b2); then \
		$(LINT) $(LFLAGS) $(LISTM32) $(INCLIST) $(DEFLIST) $(ODEFLIST) $(LIBS) $(CFILES) ; \
	else \
	    if (i386); then \
		$(LINT) $(LFLAGS) $(LISTI386) $(INCLIST) $(DEFLIST) $(ODEFLIST) $(LIBS) $(CFILES) ; \
	    fi \
	   fi \
	fi
