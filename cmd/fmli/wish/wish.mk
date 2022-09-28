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
#ident	"@(#)fmli:wish/wish.mk	1.34"
#


INC=$(ROOT)/usr/include
#USRLIB=$(CCSROOT)/usr/ccs/lib
LIBRARY=libwish.a
CFLAGS= -O
AR=	ar
#DEFS = -DRELEASE='"FMLI Release 4.0 Load K9"'
HEADER1=../inc
CURSES_H=$(INC)
INCLUDE=	-I$(HEADER1) -I$(CURSES_H)
CFLAGS= -O

$(LIBRARY): \
		$(LIBRARY)(browse.o) \
		$(LIBRARY)(display.o) \
		$(LIBRARY)(error.o) \
		$(LIBRARY)(flush.o) \
		$(LIBRARY)(getstring.o) \
		$(LIBRARY)(global.o) \
		$(LIBRARY)(mudge.o) \
		$(LIBRARY)(objop.o) \
		$(LIBRARY)(stubs.o) \
		$(LIBRARY)(virtual.o) \
		$(LIBRARY)(wdwcreate.o) \
		$(LIBRARY)(wdwlist.o) \
		$(LIBRARY)(wdwmgmt.o)

$(LIBRARY)(browse.o): $(HEADER1)/actrec.h
$(LIBRARY)(browse.o): $(HEADER1)/ctl.h
$(LIBRARY)(browse.o): $(HEADER1)/moremacros.h
$(LIBRARY)(browse.o): $(HEADER1)/slk.h
$(LIBRARY)(browse.o): $(HEADER1)/terror.h
$(LIBRARY)(browse.o): $(HEADER1)/token.h
$(LIBRARY)(browse.o): $(HEADER1)/wish.h
$(LIBRARY)(browse.o): browse.c

$(LIBRARY)(display.o): $(HEADER1)/sizes.h
$(LIBRARY)(display.o): $(HEADER1)/typetab.h
$(LIBRARY)(display.o): $(HEADER1)/wish.h
$(LIBRARY)(display.o): display.c

$(LIBRARY)(error.o): $(HEADER1)/token.h
$(LIBRARY)(error.o): $(HEADER1)/wish.h
$(LIBRARY)(error.o): error.c

$(LIBRARY)(flush.o): $(HEADER1)/wish.h
$(LIBRARY)(flush.o): flush.c

$(LIBRARY)(getstring.o): $(HEADER1)/actrec.h
$(LIBRARY)(getstring.o): $(HEADER1)/moremacros.h
$(LIBRARY)(getstring.o): $(HEADER1)/slk.h
$(LIBRARY)(getstring.o): $(HEADER1)/token.h
$(LIBRARY)(getstring.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(getstring.o): $(HEADER1)/winp.h
$(LIBRARY)(getstring.o): $(HEADER1)/wish.h
$(LIBRARY)(getstring.o): getstring.c

$(LIBRARY)(global.o): $(HEADER1)/actrec.h
$(LIBRARY)(global.o): $(HEADER1)/ctl.h
$(LIBRARY)(global.o): $(HEADER1)/message.h
$(LIBRARY)(global.o): $(HEADER1)/moremacros.h
$(LIBRARY)(global.o): $(HEADER1)/slk.h
$(LIBRARY)(global.o): $(HEADER1)/terror.h
$(LIBRARY)(global.o): $(HEADER1)/token.h
$(LIBRARY)(global.o): $(HEADER1)/wish.h
$(LIBRARY)(global.o): global.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $(DEFS) global.c
	$(AR) rv $@ global.o	 
	/bin/rm -f global.o


$(LIBRARY)(mudge.o): $(HEADER1)/actrec.h
$(LIBRARY)(mudge.o): $(HEADER1)/ctl.h
$(LIBRARY)(mudge.o): $(HEADER1)/moremacros.h
$(LIBRARY)(mudge.o): $(HEADER1)/slk.h
$(LIBRARY)(mudge.o): $(HEADER1)/token.h
$(LIBRARY)(mudge.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(mudge.o): $(HEADER1)/wish.h
$(LIBRARY)(mudge.o): mudge.c

$(LIBRARY)(objop.o): $(HEADER1)/message.h
$(LIBRARY)(objop.o): $(HEADER1)/moremacros.h
$(LIBRARY)(objop.o): $(HEADER1)/optabdefs.h
$(LIBRARY)(objop.o): $(HEADER1)/procdefs.h
$(LIBRARY)(objop.o): $(HEADER1)/terror.h
$(LIBRARY)(objop.o): $(HEADER1)/typetab.h
$(LIBRARY)(objop.o): $(HEADER1)/wish.h
$(LIBRARY)(objop.o): $(HEADER1)/sizes.h
$(LIBRARY)(objop.o): objop.c

$(LIBRARY)(stubs.o): $(HEADER1)/token.h
$(LIBRARY)(stubs.o): $(HEADER1)/wish.h
$(LIBRARY)(stubs.o): stubs.c

$(LIBRARY)(virtual.o): $(HEADER1)/actrec.h
$(LIBRARY)(virtual.o): $(HEADER1)/message.h
$(LIBRARY)(virtual.o): $(HEADER1)/moremacros.h
$(LIBRARY)(virtual.o): $(HEADER1)/slk.h
$(LIBRARY)(virtual.o): $(HEADER1)/token.h
$(LIBRARY)(virtual.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(virtual.o): $(HEADER1)/wish.h
$(LIBRARY)(virtual.o): virtual.c

$(LIBRARY)(wdwcreate.o): $(HEADER1)/actrec.h
$(LIBRARY)(wdwcreate.o): $(HEADER1)/ctl.h
$(LIBRARY)(wdwcreate.o): $(HEADER1)/menudefs.h
$(LIBRARY)(wdwcreate.o): $(HEADER1)/slk.h
$(LIBRARY)(wdwcreate.o): $(HEADER1)/sizes.h
$(LIBRARY)(wdwcreate.o): $(HEADER1)/terror.h
$(LIBRARY)(wdwcreate.o): $(HEADER1)/token.h
$(LIBRARY)(wdwcreate.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(wdwcreate.o): $(HEADER1)/wish.h
$(LIBRARY)(wdwcreate.o): $(HEADER1)/sizes.h
$(LIBRARY)(wdwcreate.o): wdwcreate.c

$(LIBRARY)(wdwlist.o): $(HEADER1)/actrec.h
$(LIBRARY)(wdwlist.o): $(HEADER1)/ctl.h
$(LIBRARY)(wdwlist.o): $(HEADER1)/menudefs.h
$(LIBRARY)(wdwlist.o): $(HEADER1)/moremacros.h
$(LIBRARY)(wdwlist.o): $(HEADER1)/slk.h
$(LIBRARY)(wdwlist.o): $(HEADER1)/terror.h
$(LIBRARY)(wdwlist.o): $(HEADER1)/token.h
$(LIBRARY)(wdwlist.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(wdwlist.o): $(HEADER1)/wish.h
$(LIBRARY)(wdwlist.o): wdwlist.c

$(LIBRARY)(wdwmgmt.o): $(HEADER1)/actrec.h
$(LIBRARY)(wdwmgmt.o): $(HEADER1)/ctl.h
$(LIBRARY)(wdwmgmt.o): $(HEADER1)/menudefs.h
$(LIBRARY)(wdwmgmt.o): $(HEADER1)/moremacros.h
$(LIBRARY)(wdwmgmt.o): $(HEADER1)/slk.h
$(LIBRARY)(wdwmgmt.o): $(HEADER1)/terror.h
$(LIBRARY)(wdwmgmt.o): $(HEADER1)/token.h
$(LIBRARY)(wdwmgmt.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(wdwmgmt.o): $(HEADER1)/wish.h
$(LIBRARY)(wdwmgmt.o): wdwmgmt.c

$(LIBRARY)(wdwsuspend.o): $(HEADER1)/actrec.h
$(LIBRARY)(wdwsuspend.o): $(HEADER1)/ctl.h
$(LIBRARY)(wdwsuspend.o): $(HEADER1)/menudefs.h
$(LIBRARY)(wdwsuspend.o): $(HEADER1)/moremacros.h
$(LIBRARY)(wdwsuspend.o): $(HEADER1)/slk.h
$(LIBRARY)(wdwsuspend.o): $(HEADER1)/terror.h
$(LIBRARY)(wdwsuspend.o): $(HEADER1)/token.h
$(LIBRARY)(wdwsuspend.o): $(HEADER1)/wish.h
$(LIBRARY)(wdwsuspend.o): wdwsuspend.c

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
