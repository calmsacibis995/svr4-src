#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sccs:lib/cassi/cassi.mk	1.14"
#
#


AR = ar
LORDER = lorder

CFLAGS = -O
INCBASE=../../hdr
CCSBIN=$(ROOT)/usr/ccs/bin
CCSLIB=$(ROOT)/usr/ccs/lib

LIBRARY = ../cassi.a

PRODUCTS = $(LIBRARY)

LINT = lint

LINTFLAGS = 

LLIBRARY = llib-lcassi.ln

SGSBASE=../../..

INC=
INCSYS=

INCLIST=-I$(SGSBASE)/sgs/inc/common

FILES = gf.o	\
	cmrcheck.o	\
	deltack.o	\
	error.o	\
	filehand.o

LFILES = gf.ln	\
	cmrcheck.ln	\
	deltack.ln	\
	error.ln	\
	filehand.ln

all: $(PRODUCTS)
	@echo "Library $(PRODUCTS) is up to date\n"

install:	$(PRODUCTS)

clean:
	-rm -f *.o
	-rm -f lint.out
	-rm -f *.ln

clobber:	clean
	-rm -f $(PRODUCTS)

.SUFFIXES : .o .c .e .r .f .y .yr .ye .l .s .ln

.c.ln:
	echo "$<:" >> lint.out
	$(LINT) $(CFLAGS) $(LINTFLAGS) $(INCLIST) $< -c >> lint.out

lintit: $(LLIBRARY)
	@echo "Library $(LLIBRARY) is up to date\n"

.c.o:
	$(CC) $(CFLAGS) $(INCLIST) -c $< 

$(LIBRARY): $(FILES)
	$(AR) cr $(LIBRARY) `$(LORDER) *.o | tsort`
	$(CH) chmod 664 $(LIBRARY)

$(LLIBRARY): $(LFILES)
	rm -f $(LLIBRARY)
	$(LINT) $(CFLAGS) $(LINTFLAGS) *.ln -o cassi >> lint.out
	ln -f $(LLIBRARY) ../$(LLIBRARY)

gf.ln:	gf.c $(INCBASE)/filehand.h
cmrcheck.ln:	cmrcheck.c $(INCBASE)/filehand.h
deltack.ln:	deltack.c $(INCBASE)/filehand.h $(INCBASE)/had.h $(INCBASE)/defines.h
filehand.ln:	filehand.c $(INCBASE)/filehand.h
error.ln:	error.c

gf.o:	gf.c	\
	 ../../hdr/filehand.h
cmrcheck.o:	cmrcheck.c	\
	 ../../hdr/filehand.h
deltack.o:	deltack.c	\
	 ../../hdr/filehand.h	\
	 ../../hdr/had.h	\
	 ../../hdr/defines.h
filehand.o:	filehand.c ../../hdr/filehand.h

