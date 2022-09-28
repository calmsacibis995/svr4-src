#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sdb:cfront/src/makefile	1.3"
CCC	=	xCC
PATCH=
# RETBUG default is to be set to circumvent C compiler bug.
# BSD and System V C compilers generate (bad) code copying a value 
#     when seeing a "return;" without a return value in a function
#     returning a structure value (simpl.c and print.c)
RETBUG=-DRETBUG
CCFLAGS=
OFILES	=	alloc.o dcl.o dcl2.o dcl3.o del.o error.o expand.o expr.o expr2.o \
		lalex.o lex.o main.o norm.o norm2.o print.o repr.o \
		simpl.o size.o table.o typ.o typ2.o y.tab.o

cfront:	$(OFILES)
	$(CCC)	-o cfront $(OFILES)

# cfront.h:	token.h typedef.h

y.tab.c:	gram.y
	yacc gram.y
	sed '/^# [fl]i[ln]e/d' <y.tab.c >y.tab.x
	sed 's/yyerror(const/yyerror(/' <y.tab.x >y.tab.c
	rm y.tab.x

alloc.o:	alloc.c cfront.h
	$(CCC) $(CCFLAGS) -c alloc.c 

dcl.o:	dcl.c cfront.h size.h
	$(CCC) $(CCFLAGS) -c dcl.c 

dcl2.o:	dcl2.c cfront.h size.h
	$(CCC) $(CCFLAGS) -c dcl2.c 

dcl3.o:	dcl3.c cfront.h size.h
	$(CCC) $(CCFLAGS) -c dcl3.c 

del.o:	del.c cfront.h
	$(CCC) $(CCFLAGS) -c del.c 

error.o:	error.c cfront.h size.h
	$(CCC) $(CCFLAGS) -c error.c 

expand.o:	expand.c cfront.h
	$(CCC) $(CCFLAGS) -c expand.c 

expr.o:	expr.c cfront.h size.h
	$(CCC) $(CCFLAGS) -c expr.c 

expr2.o:	expr2.c cfront.h size.h
	$(CCC) $(CCFLAGS) -c expr2.c 

lalex.o:	lalex.c cfront.h tqueue.h yystype.h
	$(CCC) $(CCFLAGS) -c lalex.c 

lex.o:	lex.c cfront.h size.h tqueue.h yystype.h
	$(CCC) $(CCFLAGS) -c lex.c 

main.o:	main.c cfront.h
	$(CCC) $(PATCH) $(CCFLAGS) -c main.c 

norm.o:	norm.c cfront.h size.h
	$(CCC) $(CCFLAGS) -c norm.c 

norm2.o:	norm2.c cfront.h size.h
	$(CCC) $(CCFLAGS) -c norm2.c 

print.o:	print.c cfront.h
	$(CCC) $(RETBUG) $(CCFLAGS) -c print.c 

repr.o:	repr.c cfront.h
	$(CCC) $(CCFLAGS) -c repr.c 

simpl.o:	simpl.c cfront.h size.h
	$(CCC) $(RETBUG) $(CCFLAGS) -c simpl.c 

size.o:	size.c cfront.h size.h
	$(CCC) $(CCFLAGS) -c size.c 

table.o:	table.c cfront.h
	$(CCC) $(CCFLAGS) -c table.c 

typ.o:	typ.c cfront.h size.h
	$(CCC) $(CCFLAGS) -c typ.c 

typ2.o:	typ2.c cfront.h size.h
	$(CCC) $(CCFLAGS) -c typ2.c 

y.tab.o:	y.tab.c cfront.h size.h
	$(CCC) -DGRAM $(CCFLAGS) -c y.tab.c 
