#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libc-port:stdio/makefile	1.14.3.3"
#
# makefile for libc/port/stdio
#
#

.SUFFIXES: .p .P .A
CC=cc
CFLAGS=-O 
PROF=
NONPROF=
PIC=
ABI=
ROOT=
INC=$(ROOT)/usr/include
INCSYS=$(ROOT)/usr/include
INCLIBC=../../inc
DEFLIST=
SDEFLIST=
INCLIST=-I$(INCLIBC)

POBJECTS=$(OBJECTS:.o=.p)

OBJECTS=\
_filbuf.o   _findbuf.o  _flsbuf.o   data.o     _wrtchk.o \
clearerr.o  ctermid.o	\
cuserid.o   doscan.o    fdopen.o	\
feof.o      ferror.o \
fgetc.o     fgets.o     fileno.o    \
flush.o     fopen.o     fpos.o      fputc.o     fputs.o	\
fread.o     fseek.o     ftell.o     fwrite.o	\
getc.o      getchar.o   getpass.o   gets.o      getw.o	\
popen.o     putchar.o   putc.o      puts.o      putw.o	\
rewind.o    scanf.o     setbuf.o    setvbuf.o	\
system.o	\
tempnam.o   tmpfile.o   tmpnam.o    ungetc.o

PIC_OBJECTS=\
_filbuf.P   _findbuf.P  _flsbuf.P   _wrtchk.P   \
clearerr.P  ctermid.P   cuserid.P   doscan.P    \
fdopen.P    feof.P      ferror.P    fgetc.P     \
fgets.P     fileno.P    flush.P     fopen.P     \
fpos.P      fputc.P     fputs.P     fread.P     fseek.P     \
ftell.P     fwrite.P    getc.P      getchar.P   \
getpass.P   gets.P      getw.P      popen.P     \
putc.P      putchar.P   puts.P      putw.P      \
rewind.P    scanf.P     setbuf.P    setvbuf.P   \
system.P    tempnam.P   tmpfile.P   tmpnam.P    \
ungetc.P

ABI_OBJECTS=$(PIC_OBJECTS:.P=.A)

nonprof: $(OBJECTS)

prof: $(POBJECTS)

pic: $(PIC_OBJECTS)

abi: $(ABI_OBJECTS)

_filbuf.o _filbuf.p  _findbuf.o _findbuf.p   \
	_flsbuf.o _flsbuf.p  data.o data.p  \
	_wrtchk.o _wrtchk.p fdopen.o fdopen.p \
	fgets.o fgets.p flush.o flush.p \
	fopen.o fopen.p \
	fputs.o fputs.p fread.o fread.p \
	fwrite.o fwrite.p gets.o gets.p \
	puts.o puts.p \
	setbuf.o setbuf.p setvbuf.o sevtbuf.p \
	ungetc.o ungetc.p: stdiom.h
doscan.o doscan.p: $(INC)/ctype.h
fopen.o fopen.p popen.o popen.p: $(INC)/fcntl.h
cuserid.o cuserid.p: $(INC)/pwd.h
getpass.o getpass.p \
	popen.o popen.p \
	system.o system.p: $(INC)/signal.h \
		$(INCSYS)/sys/signal.h
_filbuf.o _filbuf.p  _findbuf.o _findbuf.p   \
	_flsbuf.o _flsbuf.p  data.o data.p  _wrtchk.o _wrtchk.p\
	clearerr.o clearerr.p ctermid.o ctermid.p \
	cuserid.o cuserid.p \
	doscan.o doscan.p fdopen.o fdopen.p \
	feof.o feof.p  ferror.o ferror.p\
	fgetc.o fgetc.p fgets.o fgets.p \
	fileno.o fileno.p \
	flush.o flush.p fopen.o fopen.p \
	fpos.o fpos.p fputc.o fputc.p fputs.o fputs.p \
	fread.o fread.p fseek.o fseek.p \
	ftell.o ftell.p fwrite.o fwrite.p \
	getc.o getc.p \
	getchar.o getchar.p getpass.o getpass.p \
	gets.o gets.p getw.o getw.p \
	popen.o popen.p putchar.o putchar.p \
	putc.o putc.p \
	puts.o puts.p putw.o putw.p \
	rewind.o rewind.p scanf.o scanf.p \
	setbuf.o setbuf.p setvbuf.o sevtbuf.p tempnam.o tempnam.p \
	tmpfile.o tmpfile.p tmpnam.o tmpnam.p \
	ungetc.o ungetc.p: $(INC)/stdio.h

_findbuf.o _findbuf.p data.o data.p \
	flush.o flush.p \
	setbuf.o setbuf.p \
	setvbuf.o setvbuf.p: $(INC)/stdlib.h

fread.o fread.p: $(INC)/stddef.h

fgets.o fgets.p fputs.o fputs.p  \
	fread.o fread.p fwrite.o fwrite.p \
	gets.o gets.p getw.o getw.p \
	puts.o puts.p putw.o putw.p: $(INC)/memory.h

ctermid.o ctermid.p cuserid.o cuserid.p \
	gets.o gets.p tempnam.o tempnam.p: $(INC)/string.h

doscan.o doscan.p popen.o popen.p : $(INCSYS)/sys/errno.h

getpass.o getpass.p: $(INC)/termio.h \
		$(INCSYS)/sys/termio.h

doscan.o doscan.p \
	fread.o fread.p fwrite.o fwrite.p: $(INC)/values.h

doscan.o doscan.p \
	scanf.o scanf.p: $(INC)/varargs.h

fgetc.o fgets.o _filbuf.o _findbuf.o \
	_flsbuf.o  fopen.o fputc.o fputs.o \
	flush.o flush.p \
	fread.o fseek.o fwrite.o \
	getchar.o gets.o getw.o \
        putchar.o puts.o putw.o \
	setbuf.o ungetc.o: $(INCLIBC)/shlib.h

.c.o .c.p .c.P .c.A:
	$(NONPROF)@echo $*.c:
	$(NONPROF)$(CC) $(DEFLIST) $(SDEFLIST) $(INCLIST) $(CFLAGS) -c $*.c
	$(PROF)@echo $*.c:
	$(PROF)$(CC) $(DEFLIST) $(INCLIST) $(CFLAGS) -c -p $*.c && mv $(*F).o $*.p
	$(PIC)@echo $*.c:
	$(PIC)$(CC) $(DEFLIST) $(INCLIST) $(CFLAGS) -c -DDSHLIB -Kpic $*.c && mv $(*F).o $*.P
	$(ABI)@echo $*.c:
	$(ABI)$(CC) $(DEFLIST) $(INCLIST) $(CFLAGS) -c -DABI -DDSHLIB -Kpic $*.c && mv $(*F).o $*.A
