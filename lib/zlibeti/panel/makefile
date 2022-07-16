#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
#ident	"@(#)sti:panel/makefile	1.12"
#

ROOT=
OWN=bin
GRP=bin

PWD=.
CC = cc
AR = ar
LINT = lint
LINTFLAGS = -u
USRLIB=$(ROOT)/usr/lib
CCSLIB=$(ROOT)/usr/ccs/lib
LIBRARY = libpanel.a
PRODUCT = $(LIBRARY) llib-lpanel.ln llib-lpanel

INC = $(ROOT)/usr/include
INCLUDE = -I $(PWD) -I $(INC)
HFILES = private.h panel.h

CFLAGS = -O

SOURCES = bottom.c	delete.c	misc.c	update.c	\
	  move.c	new.c		top.c	replace.c

OBJ =	$(LIBRARY)(bottom.o)	$(LIBRARY)(delete.o)	$(LIBRARY)(misc.o)	\
	$(LIBRARY)(move.o)	$(LIBRARY)(new.o)	$(LIBRARY)(top.o) \
	$(LIBRARY)(update.o)	$(LIBRARY)(replace.o)

.c.a:
	$(CC) $(INCLUDE) $(CFLAGS) -c $<
	$(AR) rv $@ $*.o
	rm -f $*.o

.PRECIOUS:	${LIBRARY}

all:	$(PRODUCT)

$(LIBRARY):	$(OBJ)

$(OBJ):	$(HFILES)

lintit:
	$(LINT) $(INCLUDE) $(LINTFLAGS) -l curses $(SOURCES)

llib-lpanel:	llib-lpanel.c
		ln llib-lpanel.c llib-lpanel

llib-lpanel.ln:	llib-lpanel.c
		$(LINT) -o panel -vx $(INCLUDE) llib-lpanel.c

install:	all
		install -f $(CCSLIB) -m 644 -u $(OWN) -g $(GRP) libpanel.a
		install -f $(CCSLIB) -m 644 -u $(OWN) -g $(GRP) llib-lpanel
		install -f $(CCSLIB) -m 644 -u $(OWN) -g $(GRP) llib-lpanel.ln
		install -f $(INC) -m 444 -u $(OWN) -g $(GRP) panel.h

clean:
		rm -f *.o

clobber:	clean
		rm -f $(PRODUCT)
