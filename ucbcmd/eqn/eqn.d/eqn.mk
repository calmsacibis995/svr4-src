#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#     Portions Copyright(c) 1988, Sun Microsystems, Inc.
#     All Rights Reserved

#ident	"@(#)ucbeqn:eqn.d/eqn.mk	1.2.3.1"

#	makefile eqn. 

OL = $(ROOT)/
CFLAGS = -O -I. -I..
YFLAGS = -d
LDFLAGS = -s $(SHLIBS)
IFLAG = -i
YACC=yacc

SOURCE= e.y e.h diacrit.c eqnbox.c font.c fromto.c funny.c glob.c integral.c \
 io.c lex.c lookup.c mark.c matrix.c move.c over.c paren.c \
 pile.c shift.c size.c sqrt.c text.c

FILES= e.o diacrit.o eqnbox.o font.o fromto.o funny.o glob.o integral.o \
 io.o lex.o lookup.o mark.o matrix.o move.o over.o paren.o \
 pile.o shift.o size.o sqrt.o text.o

INSDIR = $(OL)usr/ucb

all:	eqn

eqn:	$(FILES)
	$(CC) $(CFLAGS) $(FILES) -o eqn $(LDFLAGS)

e.c:	e.def

e.def:	../e.y
	$(YACC) -d ../e.y
	mv y.tab.c e.c
	mv y.tab.h e.def

e.o:	e.c ../e.h
	$(CC) $(CFLAGS) -c e.c
diacrit.o:	../diacrit.c ../e.h
	$(CC) $(CFLAGS) -c ../diacrit.c
eqnbox.o:	../eqnbox.c ../e.h
	$(CC) $(CFLAGS) -c ../eqnbox.c
font.o:	../font.c ../e.h
	$(CC) $(CFLAGS) -c ../font.c
fromto.o:	../fromto.c ../e.h
	$(CC) $(CFLAGS) -c ../fromto.c
funny.o:	../funny.c ../e.h
	$(CC) $(CFLAGS) -c ../funny.c
glob.o:	../glob.c ../e.h
	$(CC) $(CFLAGS) -c ../glob.c
integral.o:	../integral.c ../e.h
	$(CC) $(CFLAGS) -c ../integral.c
io.o:	../io.c ../e.h
	$(CC) $(CFLAGS) -c ../io.c
lex.o:	../lex.c ../e.h
	$(CC) $(CFLAGS) -c ../lex.c
lookup.o:	../lookup.c ../e.h
	$(CC) $(CFLAGS) -c ../lookup.c
mark.o:	../mark.c ../e.h
	$(CC) $(CFLAGS) -c ../mark.c
matrix.o:	../matrix.c ../e.h
	$(CC) $(CFLAGS) -c ../matrix.c
move.o:	../move.c ../e.h
	$(CC) $(CFLAGS) -c ../move.c
over.o:	../over.c ../e.h
	$(CC) $(CFLAGS) -c ../over.c
paren.o: ../paren.c ../e.h
	$(CC) $(CFLAGS) -c ../paren.c
pile.o:	../pile.c ../e.h
	$(CC) $(CFLAGS) -c ../pile.c
shift.o: ../shift.c ../e.h
	$(CC) $(CFLAGS) -c ../shift.c
size.o:	../size.c ../e.h
	$(CC) $(CFLAGS) -c ../size.c
sqrt.o:	../sqrt.c ../e.h
	$(CC) $(CFLAGS) -c ../sqrt.c
text.o:	../text.c ../e.h
	$(CC) $(CFLAGS) -c ../text.c

install: all
	install -f $(INSDIR) -u bin -g bin -m 00555 eqn

clean:	
	rm -f $(FILES) e.c e.def 

clobber: clean
	rm -f eqn
