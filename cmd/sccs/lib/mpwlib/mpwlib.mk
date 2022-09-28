#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sccs:lib/mpwlib/mpwlib.mk	1.14"
#
#

AR = ar
LORDER = lorder

CFLAGS = -O

CCSBIN=$(ROOT)/usr/ccs/bin
CCSLIB=$(ROOT)/usr/ccs/lib

LIBRARY = ../mpwlib.a

PRODUCTS = $(LIBRARY)

LLIBRARY = llib-lmpw.ln

LINT = lint

LINTFLAGS =

ROOT =

SGSBASE=../../..
INC=
INCSYS=

INCLIST=-I$(SGSBASE)/sgs/inc/common

FILES =	abspath.o	\
	any.o		\
	cat.o		\
	clean.o		\
	dname.o		\
	fatal.o		\
	fdfopen.o	\
	fmalloc.o	\
	imatch.o	\
	index.o		\
	lockit.o	\
	logname.o	\
	patoi.o		\
	rename.o	\
	repl.o		\
	satoi.o		\
	setsig.o	\
	sname.o		\
	strend.o	\
	trnslat.o	\
	userexit.o	\
	xcreat.o	\
	xlink.o		\
	xopen.o		\
	xpipe.o		\
	xunlink.o	\
	xmsg.o		\
	zero.o

LFILES =abspath.ln	\
	any.ln		\
	cat.ln		\
	clean.ln	\
	dname.ln	\
	fatal.ln	\
	fdfopen.ln	\
	fmalloc.ln	\
	imatch.ln	\
	index.ln	\
	lockit.ln	\
	logname.ln	\
	patoi.ln	\
	rename.ln	\
	repl.ln		\
	satoi.ln	\
	setsig.ln	\
	sname.ln	\
	strend.ln	\
	trnslat.ln	\
	userexit.ln	\
	xcreat.ln	\
	xlink.ln	\
	xopen.ln	\
	xpipe.ln	\
	xunlink.ln	\
	xmsg.ln		\
	zero.ln

all: $(LIBRARY)
	@echo "Library $(PRODUCTS) is up to date"

$(LIBRARY):	$(FILES)
	$(AR) cr $(LIBRARY) `$(LORDER) *.o | tsort`
	$(CH) chmod 664 $(LIBRARY)

lintit: $(LLIBRARY)
	@echo "Library $(LLIBRARY) is up to date"

$(LLIBRARY):	$(LFILES)
	rm -f $(LLIBRARY)
	$(LINT) $(CFLAGS) $(LINTFLAGS) *.ln -o mpw >> lint.out
	ln -f $(LLIBRARY) ../$(LLIBRARY)

install:	$(LIBRARY)

clean:
	-rm -f *.o
	-rm -f *.ln
	-rm -f lint.out

clobber:	clean
	-rm -f $(PRODUCTS)

.SUFFIXES : .o .c .e .r .f .y .yr .ye .l .s .ln

.c.ln:
	echo "$<:" >> lint.out
	$(LINT) $(CFLAGS) $(INCLIST) $(LINTFLAGS) $< -c >> lint.out

.c.o:
	$(CC) $(INCLIST) $(CFLAGS) -c $< 

abspath.ln:	abspath.c
any.ln:		any.c
cat.ln:		cat.c
clean.ln:	clean.c
dname.ln:	dname.c
fatal.ln:	fatal.c
fdfopen.ln:	fdfopen.c
fmalloc.ln:	fmalloc.c
imatch.ln:	imatch.c
index.ln:	index.c
lockit.ln:	lockit.c
logname.ln:	logname.c
patoi.ln:	patoi.c
rename.ln:	rename.c
repl.ln	:	repl.c
satoi.ln:	satoi.c
setsig.ln:	setsig.c
sname.ln:	sname.c
strend.ln:	strend.c
trnslat.ln:	trnslat.c
userexit.ln:	userexit.c
xcreat.ln:	xcreat.c
xlink.ln:	xlink.c
xopen.ln:	xopen.c
xpipe.ln:	xpipe.c
xunlink.ln:	xunlink.c
xmsg.ln:	xmsg.c
zero.ln:	zero.c

abspath.o:	abspath.c
any.o:		any.c
cat.o:		cat.c
clean.o:	clean.c
dname.o:	dname.c
fatal.o:	fatal.c
fdfopen.o:	fdfopen.c
fmalloc.o:	fmalloc.c
imatch.o:	imatch.c
index.o:	index.c
lockit.o:	lockit.c
logname.o:	logname.c
patoi.o:	patoi.c
rename.o:	rename.c
repl.o	:	repl.c
satoi.o:	satoi.c
setsig.o:	setsig.c
sname.o:	sname.c
strend.o:	strend.c
trnslat.o:	trnslat.c
userexit.o:	userexit.c
xcreat.o:	xcreat.c
xlink.o:	xlink.c
xopen.o:	xopen.c
xpipe.o:	xpipe.c
xunlink.o:	xunlink.c
xmsg.o:		xmsg.c
zero.o:		zero.c


move.ln:	
	if vax; then	\
		$(CC) -c vax/move.s	; \
	elif u3b; then 	\
		$(CC) -c $(CFLAGS) u3b/move.c	; \
	elif u3b15 || u3b2; then	\
		$(CC) -c $(CFLAGS) u3b2/move.c	; \
	elif pdp11; then	\
		$(CC) -c $(CFLAGS) pdp11/move.c	; \
	else	\
		$(CC) -c $(CFLAGS) u370/move.c	; \
	fi

alloca.ln:
	if vax; then	\
		$(CC) -c vax/alloca.s	; \
	elif u3b; then 	\
		$(CC) -c u3b/alloca.s	; \
	elif u3b15 || u3b2; then	\
		$(CC)  -c u3b2/alloca.s	; \
	elif pdp11; then	\
		$(CC) -c pdp11/alloca.s	; \
	else	\
		$(CC) -c $(CFLAGS) u370/alloca.c	; \
	fi

move.o:	
	if vax; then	\
		$(CC) -c vax/move.s	; \
	elif u3b; then 	\
		$(CC) -c $(CFLAGS) u3b/move.c	; \
	elif u3b15 || u3b2; then	\
		$(CC) -c $(CFLAGS) u3b2/move.c	; \
	elif pdp11; then	\
		$(CC) -c $(CFLAGS) pdp11/move.c	; \
	else	\
		$(CC) -c $(CFLAGS) u370/move.c	; \
	fi

alloca.o:
	if vax; then	\
		$(CC) -c vax/alloca.s	; \
	elif u3b; then 	\
		$(CC) -c u3b/alloca.s	; \
	elif u3b15 || u3b2; then	\
		$(CC)  -c u3b2/alloca.s	; \
	elif pdp11; then	\
		$(CC) -c pdp11/alloca.s	; \
	else	\
		$(CC) -c $(CFLAGS) u370/alloca.c	; \
	fi

.PRECIOUS:	$(PRODUCTS)
