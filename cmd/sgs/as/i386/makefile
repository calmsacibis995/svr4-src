#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)as:i386/makefile	1.6"

#
#	i386 Assembler Makefile
#

ROOT=
SGS=i386
OWN=bin
GRP=bin
CC=cc
CFLAGS=-c -O
LINT=lint
LINTFLAGS=
LINK_MODE=
YACC= 
YFLAGS	= -d

CCSBIN=$(ROOT)/usr/ccs/bin
CCSLIB=$(ROOT)/usr/ccs/lib
INC=$(ROOT)/usr/include
INCSYS=$(ROOT)/usr/include
LIB=$(ROOT)/lib
SGSBASE=../..
MACH=i386
MACHINC=$(SGSBASE)/inc/$(MACH)
COMINC=$(SGSBASE)/inc/common
LIBELF=$(SGSBASE)/libelf/$(MACH)/libelf.a
COMDIR=../common
LDLIBS=

INS=$(SGSBASE)/sgs.install
INSDIR=$(ROOT)/bin

STRIP=strip

ARCH=AR32WR
FLEX= -DFLEXNAMES

DEFLIST=-D$(ARCH) $(FLEX) -Di386 
INCLIST=-I. -I$(COMDIR) -I$(COMINC) -I$(MACHINC) -I$(INC)
ENV=


CC_CMD=$(CC) -c $(CFLAGS) $(DEFLIST) $(INCLIST) $(ENV)
LD_CMD=$(CC) $(LDFLAGS) 
STRIP_CMD=$(STRIP)
LINT_CMD=$(LINT) -c $(CFLAGS) $(DEFLIST) $(INCLIST) $(ENV)

LDFLAGS=$(FFLAG)

LIBM=-lm

o=o

HFILES = expand2.h gendefs.h instab.h instr.h ops.out \
	symbols2.h systems.h \
	$(COMDIR)/codeout.h $(COMDIR)/expand.h $(COMDIR)/expr.h \
	$(COMDIR)/section.h $(COMDIR)/symbols.h \
	$(COMINC)/elf.h $(COMINC)/libelf.h  \
	$(MACHINC)/paths.h $(MACHINC)/sgs.h


SOURCES = addr2.c  expand2.c float.c gencode.c instab.c instr.c \
	$(COMDIR)/addr1.c $(COMDIR)/code.c $(COMDIR)/codeout.c \
	$(COMDIR)/errors.c $(COMDIR)/expand1.c $(COMDIR)/expr.c \
	$(COMDIR)/obj.c \
	$(COMDIR)/pass0.c $(COMDIR)/pass1.c $(COMDIR)/pass2.c \
	$(COMDIR)/symbols.c $(COMDIR)/symlist.c

OBJS =	addr2.$o expand2.$o float.$o gencode.$o instr.$o parse.$o \
	addr1.$o code.$o codeout.$o \
	errors.$o expand1.$o expr.$o \
	instab.$o obj.$o \
	pass0.$o pass1.$o pass2.$o \
	symbols.$o symlist.$o

PRODUCTS = as

.SUFFIXES: .ln

.c.ln:
	$(LINT_CMD) $<

.c.o:
	$(CC_CMD) $<

#################################################################
#
# EXTERNAL TARGETS:
#
#	all:		build the assembler in this directory
#	as:		build the assembler in this directory
#	lintit:		lint the assembler
#	install:	install the assembler and m4 files in
#			CCSBIN and CCSLIB owned by OWN/GRP
#	clean:		remove all intermediate files
#	shrink:		remove all intermediate files and "as"
#	clobber:	remove all intermediate files and "as"
#
#################################################################

all: $(PRODUCTS)


lintit: $(SOURCES) $(HFILES) parse.c parse.h
	$(LINT) $(LINTFLAGS) $(DEFLIST) $(INCLIST) $(SOURCES) parse.c > lint.out 

install:	$(CCSBIN)/$(SGS)as $(CCSLIB)/cm4defs

clean:
	-rm -f $(OBJS:$o=o) $(OBJS:$o=ln)
	-rm -f lint.out parse.h parse.c

shrink clobber:	clean 
	-rm -f as


#################################################################
#	INTERNAL TARGETS
#################################################################

$(PRODUCTS):  $(OBJS)
	$(LD_CMD) -o as $(OBJS) $(LINK_MODE) $(LIBM) $(LIBELF) $(LDLIBS) 


$(CCSBIN)/$(SGS)as:	all
	cp as as.bak
	-$(STRIP_CMD) as
	sh $(INS) 755 $(OWN) $(GRP) $(CCSBIN)/$(SGS)as as
	mv as.bak as

$(CCSLIB)/cm4defs:	cm4defs
	sh $(SGSBASE)/sgs.install 664 $(OWN) $(GRP) $(CCSLIB)/cm4defs cm4defs

#########################################################################

addr1.$o:	$(COMDIR)/addr1.c
		$(CC_CMD) $(COMDIR)/addr1.c

code.$o:	$(COMDIR)/code.c
		$(CC_CMD) $(COMDIR)/code.c

codeout.$o:	$(COMDIR)/codeout.c
		$(CC_CMD) $(COMDIR)/codeout.c

errors.$o:	$(COMDIR)/errors.c
		$(CC_CMD) $(COMDIR)/errors.c

expand1.$o:	$(COMDIR)/expand1.c
		$(CC_CMD) $(COMDIR)/expand1.c

expr.$o:	$(COMDIR)/expr.c
		$(CC_CMD) $(COMDIR)/expr.c


obj.$o:		$(COMDIR)/obj.c
		$(CC_CMD) $(COMDIR)/obj.c

pass0.$o:	$(COMDIR)/pass0.c
		$(CC_CMD) $(COMDIR)/pass0.c

pass1.$o:	$(COMDIR)/pass1.c
		$(CC_CMD) $(COMDIR)/pass1.c

pass2.$o:	$(COMDIR)/pass2.c
		$(CC_CMD) $(COMDIR)/pass2.c 

symbols.$o:	$(COMDIR)/symbols.c
		$(CC_CMD) $(COMDIR)/symbols.c

symlist.$o:	$(COMDIR)/symlist.c
		$(CC_CMD) $(COMDIR)/symlist.c



addr2.$o:	addr2.c
expand2.$o:	expand2.c
float.$o:	float.c
gencode.$o:	gencode.c
instab.$o:	instab.c
instr.$o:	instr.c
parse.$o:	parse.c


#########################################
#	HEADER FILE DEPENDENCIES	#
#########################################

#	DIRECTORY common/

addr1.$o:	gendefs.h
addr1.$o:	systems.h
addr1.$o:	$(COMDIR)/symbols.h
addr1.$o:	$(COMDIR)/codeout.h
addr1.$o:	$(COMINC)/libelf.h

code.$o:	gendefs.h
code.$o:	systems.h
code.$o:	$(COMDIR)/symbols.h
code.$o:	$(COMDIR)/codeout.h
code.$o:	$(COMDIR)/expr.h
code.$o:	$(COMDIR)/section.h
code.$o:	$(COMINC)/elf.h

codeout.$o:	gendefs.h
codeout.$o:	systems.h
codeout.$o:	expand2.h
codeout.$o:	$(COMDIR)/expand.h
codeout.$o:	$(COMDIR)/symbols.h
codeout.$o:	$(COMDIR)/codeout.h
codeout.$o:	$(COMDIR)/section.h
codeout.$o:	$(COMDIR)/expr.h
codeout.$o:	$(COMINC)/libelf.h
codeout.$o:	$(COMINC)/elf.h

errors.$o:	gendefs.h
errors.$o:	systems.h

expand1.$o:	systems.h
expand1.$o:	expand2.h
expand1.$o:	$(COMDIR)/expand.h
expand1.$o:	$(COMDIR)/symbols.h
expand1.$o:	$(COMDIR)/codeout.h
expand1.$o:	$(COMDIR)/section.h

expr.$o:	gendefs.h
expr.$o:	$(COMDIR)/expr.h
expr.$o:	$(COMDIR)/symbols.h
expr.$o:	$(COMDIR)/codeout.h
expr.$o:	$(COMDIR)/section.h
expr.$o:	instab.h


instab.$o:	ops.out

obj.$o:		instab.h
obj.$o:		gendefs.h
obj.$o:		systems.h
obj.$o:		$(COMDIR)/expr.h
obj.$o:		$(COMDIR)/codeout.h
obj.$o:		$(COMDIR)/section.h
obj.$o:		$(COMDIR)/symbols.h
obj.$o:		$(COMDIR)/codeout.h
obj.$o:		$(COMINC)/filehdr.h
obj.$o:		$(COMINC)/libelf.h
obj.$o:		$(COMINC)/elf.h
obj.$o:		$(MACHINC)/sgs.h

pass0.$o:	$(MACHINC)/paths.h
pass0.$o:	$(MACHINC)/sgs.h
pass0.$o:	gendefs.h
pass0.$o:	systems.h

pass1.$o:	$(COMDIR)/codeout.h
pass1.$o:	$(COMDIR)/section.h
pass1.$o:	$(COMDIR)/symbols.h
pass1.$o:	$(MACHINC)/paths.h
pass1.$o:	gendefs.h
pass1.$o:	systems.h

pass2.$o:	systems.h
pass2.$o:	gendefs.h
pass2.$o:	$(COMDIR)/codeout.h
pass2.$o:	$(COMDIR)/section.h
pass2.$o:	$(COMDIR)/symbols.h
pass2.$o:	$(COMINC)/libelf.h
pass2.$o:	$(COMINC)/elf.h

symbols.$o:	gendefs.h
symbols.$o:	symbols2.h
symbols.$o:	systems.h
symbols.$o:	$(COMDIR)/symbols.h

symlist.$o:	gendefs.h
symlist.$o:	systems.h
symlist.$o:	$(COMDIR)/symbols.h


#	DIRECTORY i386/

addr2.$o:	$(COMDIR)/expr.h
addr2.$o:	systems.h
addr2.$o:	gendefs.h
addr2.$o:	expand2.h
addr2.$o:	$(COMDIR)/symbols.h
addr2.$o:	$(COMDIR)/section.h
addr2.$o:	$(COMDIR)/codeout.h
addr2.$o:	$(COMDIR)/expand.h
addr2.$o:	$(COMINC)/libelf.h
addr2.$o:	$(COMINC)/elf.h

expand2.$o:	expand2.h

float.$o:	systems.h
float.$o:	instab.h
float.$o:	parse.h
float.$o:	$(COMDIR)/expr.h
float.$o:	$(COMDIR)/symbols.h

gencode.$o:	instab.h
gencode.$o:	systems.h
gencode.$o:	parse.h
gencode.$o:	gendefs.h
gencode.$o:	expand2.h
gencode.$o:	$(COMDIR)/expr.h
gencode.$o:	$(COMDIR)/symbols.h
gencode.$o:	$(COMDIR)/expand.h

instr.$o:	$(COMDIR)/symbols.h
instr.$o:	systems.h
instr.$o:	symbols2.h
instr.$o:	instr.h

parse.$o:	gendefs.h
parse.$o:	instab.h
parse.$o:	instr.h
parse.$o:	systems.h
parse.$o:	$(COMDIR)/expr.h
parse.$o:	$(COMDIR)/symbols.h
parse.$o:	$(COMDIR)/section.h



#########################################
#	MISC FILE DEPENDENCIES	#
#########################################


parse.c parse.h : parse.y 
		$(YACC) $(YFLAGS) parse.y
		mv y.tab.c parse.c
		mv y.tab.h parse.h

