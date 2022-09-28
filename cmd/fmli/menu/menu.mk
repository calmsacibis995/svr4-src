#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#
# Copyright  (c) 1985 AT&T
#	All Rights Reserved
#
#ident	"@(#)fmli:menu/menu.mk	1.12"
#


INC=$(ROOT)/usr/include
#USRLIB=$(CCSROOT)/usr/ccs/lib
#USRLIB=$(ROOT)/usr/lib
LIBRARY=libmenu.a
CFLAGS= -O
HEADER1=../inc
CURSES_H=$(INC)
INCLUDE=	-I$(HEADER1) -I$(CURSES_H)
AR=	ar



$(LIBRARY): \
		$(LIBRARY)(mclose.o) \
		$(LIBRARY)(mctl.o) \
		$(LIBRARY)(mcurrent.o) \
		$(LIBRARY)(mcustom.o) \
		$(LIBRARY)(mfolder.o) \
		$(LIBRARY)(mdefault.o) \
		$(LIBRARY)(mreshape.o) \
		$(LIBRARY)(stmenu.o)

$(LIBRARY)(mclose.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(mclose.o): $(HEADER1)/wish.h
$(LIBRARY)(mclose.o): $(HEADER1)/menu.h
$(LIBRARY)(mclose.o): mclose.c

$(LIBRARY)(mctl.o): $(HEADER1)/ctl.h
$(LIBRARY)(mctl.o): $(HEADER1)/menudefs.h
$(LIBRARY)(mctl.o): $(HEADER1)/wish.h
$(LIBRARY)(mctl.o): $(HEADER1)/menu.h
$(LIBRARY)(mctl.o): mctl.c

$(LIBRARY)(mcurrent.o): $(HEADER1)/attrs.h
$(LIBRARY)(mcurrent.o): $(HEADER1)/color_pair.h
$(LIBRARY)(mcurrent.o): $(HEADER1)/ctl.h
$(LIBRARY)(mcurrent.o): $(HEADER1)/menudefs.h
$(LIBRARY)(mcurrent.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(mcurrent.o): $(HEADER1)/wish.h
$(LIBRARY)(mcurrent.o): $(HEADER1)/menu.h
$(LIBRARY)(mcurrent.o): mcurrent.c

$(LIBRARY)(mcustom.o): $(HEADER1)/ctl.h
$(LIBRARY)(mcustom.o): $(HEADER1)/menudefs.h
$(LIBRARY)(mcustom.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(mcustom.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(mcustom.o): $(HEADER1)/wish.h
$(LIBRARY)(mcustom.o): $(HEADER1)/menu.h
$(LIBRARY)(mcustom.o): mcustom.c

$(LIBRARY)(mfolder.o): $(HEADER1)/ctl.h
$(LIBRARY)(mfolder.o): $(HEADER1)/menudefs.h
$(LIBRARY)(mfolder.o): $(HEADER1)/terror.h
$(LIBRARY)(mfolder.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(mfolder.o): $(HEADER1)/wish.h
$(LIBRARY)(mfolder.o): $(HEADER1)/sizes.h
$(LIBRARY)(mfolder.o): $(HEADER1)/menu.h
$(LIBRARY)(mfolder.o): mfolder.c

$(LIBRARY)(mdefault.o): $(HEADER1)/ctl.h
$(LIBRARY)(mdefault.o): $(HEADER1)/menudefs.h
$(LIBRARY)(mdefault.o): $(HEADER1)/terror.h
$(LIBRARY)(mdefault.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(mdefault.o): $(HEADER1)/wish.h
$(LIBRARY)(mdefault.o): $(HEADER1)/sizes.h
$(LIBRARY)(mdefault.o): $(HEADER1)/menu.h
$(LIBRARY)(mdefault.o): mdefault.c

$(LIBRARY)(mreshape.o): $(HEADER1)/ctl.h
$(LIBRARY)(mreshape.o): $(HEADER1)/menudefs.h
$(LIBRARY)(mreshape.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(mreshape.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(mreshape.o): $(HEADER1)/wish.h
$(LIBRARY)(mreshape.o): $(HEADER1)/menu.h
$(LIBRARY)(mreshape.o): mreshape.c

$(LIBRARY)(stmenu.o): $(HEADER1)/ctl.h
$(LIBRARY)(stmenu.o): $(HEADER1)/menudefs.h
$(LIBRARY)(stmenu.o): $(HEADER1)/message.h
$(LIBRARY)(stmenu.o): $(HEADER1)/moremacros.h
$(LIBRARY)(stmenu.o): $(HEADER1)/token.h
$(LIBRARY)(stmenu.o): $(HEADER1)/sizes.h
$(LIBRARY)(stmenu.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(stmenu.o): $(HEADER1)/wish.h
$(LIBRARY)(stmenu.o): $(HEADER1)/menu.h
$(LIBRARY)(stmenu.o): stmenu.c

.c.a:
	$(CC) -c $(CFLAGS) $(INCLUDE) $<
	$(AR) rv $@ $*.o
	/bin/rm -f $*.o

###### Standard makefile targets #######

all:		$(LIBRARY)

install:	all

clean:
		/bin/rm -f *.o

clobber:	clean
		/bin/rm -f $(LIBRARY)

.PRECIOUS:	$(LIBRARY)
