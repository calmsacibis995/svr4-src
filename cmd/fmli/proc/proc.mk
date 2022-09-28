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
#ident	"@(#)fmli:proc/proc.mk	1.9"
#

INC=$(ROOT)/usr/include
#USRLIB=$(CCSROOT)/usr/ccs/lib
#USRLIB=$(ROOT)/usr/lib
LIBRARY = libproc.a
HEADER1=../inc
CURSES_H=$(INC)
INCLUDE=	-I$(HEADER1) -I$(CURSES_H)
AR=		ar
CFLAGS= 	-O



$(LIBRARY):	\
		$(LIBRARY)(pclose.o) \
		$(LIBRARY)(pcurrent.o) \
		$(LIBRARY)(pctl.o) \
		$(LIBRARY)(pdefault.o) \
		$(LIBRARY)(list.o) \
		$(LIBRARY)(pnoncur.o) \
		$(LIBRARY)(open.o) \
		$(LIBRARY)(suspend.o)

$(LIBRARY)(list.o): $(HEADER1)/actrec.h
$(LIBRARY)(list.o): $(HEADER1)/ctl.h
$(LIBRARY)(list.o): $(HEADER1)/menudefs.h
$(LIBRARY)(list.o): $(HEADER1)/procdefs.h
$(LIBRARY)(list.o): $(HEADER1)/slk.h
$(LIBRARY)(list.o): $(HEADER1)/terror.h
$(LIBRARY)(list.o): $(HEADER1)/token.h
$(LIBRARY)(list.o): $(HEADER1)/wish.h
$(LIBRARY)(list.o): ./proc.h
$(LIBRARY)(list.o): list.c

$(LIBRARY)(open.o): $(HEADER1)/actrec.h
$(LIBRARY)(open.o): $(HEADER1)/moremacros.h
$(LIBRARY)(open.o): $(HEADER1)/slk.h
$(LIBRARY)(open.o): $(HEADER1)/terror.h
$(LIBRARY)(open.o): $(HEADER1)/token.h
$(LIBRARY)(open.o): $(HEADER1)/wish.h
$(LIBRARY)(open.o): ./proc.h
$(LIBRARY)(open.o): open.c

$(LIBRARY)(pclose.o): $(HEADER1)/actrec.h
$(LIBRARY)(pclose.o): $(HEADER1)/procdefs.h
$(LIBRARY)(pclose.o): $(HEADER1)/slk.h
$(LIBRARY)(pclose.o): $(HEADER1)/terror.h
$(LIBRARY)(pclose.o): $(HEADER1)/token.h
$(LIBRARY)(pclose.o): $(HEADER1)/wish.h
$(LIBRARY)(pclose.o): ./proc.h
$(LIBRARY)(pclose.o): pclose.c

$(LIBRARY)(pctl.o): $(HEADER1)/actrec.h
$(LIBRARY)(pctl.o): $(HEADER1)/ctl.h
$(LIBRARY)(pctl.o): $(HEADER1)/procdefs.h
$(LIBRARY)(pctl.o): $(HEADER1)/slk.h
$(LIBRARY)(pctl.o): $(HEADER1)/terror.h
$(LIBRARY)(pctl.o): $(HEADER1)/token.h
$(LIBRARY)(pctl.o): $(HEADER1)/wish.h
$(LIBRARY)(pctl.o): ./proc.h
$(LIBRARY)(pctl.o): pctl.c

$(LIBRARY)(pcurrent.o): $(HEADER1)/actrec.h
$(LIBRARY)(pcurrent.o): $(HEADER1)/procdefs.h
$(LIBRARY)(pcurrent.o): $(HEADER1)/slk.h
$(LIBRARY)(pcurrent.o): $(HEADER1)/sizes.h
$(LIBRARY)(pcurrent.o): $(HEADER1)/terror.h
$(LIBRARY)(pcurrent.o): $(HEADER1)/token.h
$(LIBRARY)(pcurrent.o): $(HEADER1)/wish.h
$(LIBRARY)(pcurrent.o): ./proc.h
$(LIBRARY)(pcurrent.o): pcurrent.c

$(LIBRARY)(pdefault.o): $(HEADER1)/actrec.h
$(LIBRARY)(pdefault.o): $(HEADER1)/procdefs.h
$(LIBRARY)(pdefault.o): $(HEADER1)/slk.h
$(LIBRARY)(pdefault.o): $(HEADER1)/terror.h
$(LIBRARY)(pdefault.o): $(HEADER1)/token.h
$(LIBRARY)(pdefault.o): $(HEADER1)/wish.h
$(LIBRARY)(pdefault.o): ./proc.h
$(LIBRARY)(pdefault.o): pdefault.c

$(LIBRARY)(pnoncur.o): $(HEADER1)/procdefs.h
$(LIBRARY)(pnoncur.o): $(HEADER1)/terror.h
$(LIBRARY)(pnoncur.o): $(HEADER1)/wish.h
$(LIBRARY)(pnoncur.o): ./proc.h
$(LIBRARY)(pnoncur.o): pnoncur.c

$(LIBRARY)(suspend.o): $(HEADER1)/wish.h
$(LIBRARY)(suspend.o): suspend.c

.c.a:
	$(CC) -c $(CFLAGS)  $(INCLUDE) $<
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
