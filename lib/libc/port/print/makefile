#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libc-port:print/makefile	1.12.4.1"
#
# makefile for libc/port/print
#
#

.SUFFIXES: .p .P
CC=cc
CFLAGS=-O
PROF=
NONPROF=
PIC=
ROOT=
INC=$(ROOT)/usr/include
INCLIBC=../../inc
DEFLIST=
SDEFLIST=
INCLIST=-I$(INCLIBC)

OBJECTS=\
doprnt.o    fprintf.o   printf.o    sprintf.o  vfprintf.o  vprintf.o  vsprintf.o

POBJECTS=$(OBJECTS:.o=.p)

PIC_OBJECTS=$(OBJECTS:.o=.P)

nonprof: $(OBJECTS)

prof: $(POBJECTS)

pic: $(PIC_OBJECTS)

abi:
	# No min-ABI objects in this makefile

doprnt.o doprnt.p: print.h
doprnt.o doprnt.p: $(INC)/ctype.h
doprnt.o doprnt.p fprintf.o fprintf.p \
	printf.o printf.p sprintf.o sprintf.p \
	vfprintf.o vfprintf.p vprintf.o vprintf.p \
	vsprintf.o vsprintf.p: $(INC)/stdio.h
doprnt.o doprnt.p \
	sprintf.o sprintf.p vsprintf.o vsprintf.p: $(INC)/values.h
doprnt.o doprnt.p fprintf.o fprintf.p \
	printf.o printf.p sprintf.o sprintf.p \
	vfprintf.o vfprintf.p vprintf.o vprintf.p \
	vsprintf.o vsprintf.p: $(INC)/varargs.h
doprnt.o fprintf.o printf.o sprintf.o : $(INCLIBC)/shlib.h
.c.o .c.p .c.P:
	$(NONPROF)@echo $*.c:
	$(NONPROF)$(CC) $(DEFLIST) $(SDEFLIST) $(INCLIST) $(CFLAGS) -c $*.c
	$(PROF)@echo $*.c:
	$(PROF)$(CC) $(DEFLIST) $(INCLIST) $(CFLAGS) -c -p $*.c && mv $(*F).o $*.p
	$(PIC)@echo $*.c:
	$(PIC)$(CC) $(DEFLIST) $(INCLIST) $(CFLAGS) -c -DDSHLIB -Kpic $*.c && mv $(*F).o $*.P
