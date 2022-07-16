#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
#ident	"@(#)sti:menu/makefile	1.15"
#

ROOT=
OWN=bin
GRP=bin

INC=$(ROOT)/usr/include
USRLIB=$(ROOT)/usr/lib
CCSLIB=$(ROOT)/usr/ccs/lib
PWD=.
LINT=lint
LINTFLAGS=-u
CFLAGS=-O
LIBRARY = libmenu.a
PRODUCT = $(LIBRARY) llib-lmenu.ln llib-lmenu
CC=cc
AR=ar

SOURCES=global.c newmenu.c newitem.c menuformat.c menumark.c menuopts.c\
        menupad.c scale.c menuwin.c menusub.c show.c affect.c itemvalue.c\
        driver.c link.c post.c itemopts.c chk.c menucursor.c\
        pattern.c visible.c terminit.c menuitems.c\
        connect.c curitem.c topitem.c menufore.c menuback.c menugrey.c\
        menuserptr.c itemusrptr.c itemcount.c

OBJ=$(LIBRARY)(global.o)	$(LIBRARY)(newmenu.o)	$(LIBRARY)(newitem.o)\
    $(LIBRARY)(menuformat.o)	$(LIBRARY)(menumark.o)	$(LIBRARY)(menuopts.o)\
    $(LIBRARY)(menupad.o) 	$(LIBRARY)(scale.o) 	$(LIBRARY)(menuwin.o)\
    $(LIBRARY)(menusub.o) 	$(LIBRARY)(show.o) 	$(LIBRARY)(affect.o)\
    $(LIBRARY)(itemvalue.o)	$(LIBRARY)(driver.o)	$(LIBRARY)(link.o)\
    $(LIBRARY)(post.o)		$(LIBRARY)(itemopts.o)	$(LIBRARY)(chk.o)\
    $(LIBRARY)(menucursor.o)	$(LIBRARY)(pattern.o)	$(LIBRARY)(visible.o)\
    $(LIBRARY)(terminit.o)	$(LIBRARY)(menuitems.o)	$(LIBRARY)(connect.o)\
    $(LIBRARY)(curitem.o)	$(LIBRARY)(topitem.o)	$(LIBRARY)(menufore.o)\
    $(LIBRARY)(menuback.o)	$(LIBRARY)(menugrey.o)	$(LIBRARY)(menuserptr.o)\
    $(LIBRARY)(itemusrptr.o)	$(LIBRARY)(itemcount.o)

INCLUDE=-I $(PWD) -I $(PWD)/.. -I $(INC)

HFILES=private.h menu.h

.PRECIOUS:	$(LIBRARY)

all:		$(PRODUCT)

.c.a:
		$(CC) -c $(CFLAGS) $(INCLUDE) $<
		$(AR) rv $@ $*.o
		rm -f $*.o

$(LIBRARY):	$(OBJ)

$(OBJ):		$(HFILES)

lintit:
		$(LINT) $(INCLUDE) $(LINTFLAGS) -l curses $(SOURCES)

llib-lmenu:	llib-lmenu.c
		ln llib-lmenu.c llib-lmenu

llib-lmenu.ln:	llib-lmenu.c
		$(LINT) -o menu -vx $(INCLUDE) llib-lmenu.c

install:	all
		install -f $(CCSLIB) -m 644 -u $(OWN) -g $(GRP) libmenu.a
		install -f $(CCSLIB) -m 644 -u $(OWN) -g $(GRP) llib-lmenu
		install -f $(CCSLIB) -m 644 -u $(OWN) -g $(GRP) llib-lmenu.ln
		install -f $(INC) -m 444 -u $(OWN) -g $(GRP) menu.h

clean:
		rm -f *.o

clobber:	clean
		rm -f $(PRODUCT)
