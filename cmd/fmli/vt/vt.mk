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
#ident	"@(#)fmli:vt/vt.mk	1.21"
#

INC=$(ROOT)/usr/include
#USRLIB=$(CCSROOT)/usr/ccs/lib
LIBRARY=libvt.a
CURSES_H=$(INC)
HEADER1=../inc
INCLUDE=	-I$(HEADER1) -I$(CURSES_H) 
CFLAGS= 	-O
AR=		ar

$(LIBRARY): \
		$(LIBRARY)(fits.o) \
		$(LIBRARY)(hide.o) \
		$(LIBRARY)(highlight.o) \
		$(LIBRARY)(indicator.o) \
		$(LIBRARY)(lp.o) \
		$(LIBRARY)(makebox.o) \
		$(LIBRARY)(message.o) \
		$(LIBRARY)(move.o) \
		$(LIBRARY)(offscreen.o) \
		$(LIBRARY)(physical.o) \
		$(LIBRARY)(redraw.o) \
		$(LIBRARY)(system.o) \
		$(LIBRARY)(vclose.o) \
		$(LIBRARY)(vcolor.o) \
		$(LIBRARY)(vcreate.o) \
		$(LIBRARY)(vctl.o) \
		$(LIBRARY)(vcurrent.o) \
		$(LIBRARY)(vdebug.o) \
		$(LIBRARY)(vflush.o) \
		$(LIBRARY)(vfork.o) \
		$(LIBRARY)(vinit.o) \
		$(LIBRARY)(vmark.o) \
		$(LIBRARY)(vreshape.o) \
		$(LIBRARY)(wclrwin.o) \
		$(LIBRARY)(wdelchar.o) \
		$(LIBRARY)(wgetchar.o) \
		$(LIBRARY)(wgo.o) \
		$(LIBRARY)(winschar.o) \
		$(LIBRARY)(wprintf.o) \
		$(LIBRARY)(wputchar.o) \
		$(LIBRARY)(wputs.o) \
		$(LIBRARY)(wreadchar.o) \
		$(LIBRARY)(wscrollwin.o) \
		$(LIBRARY)(showmail.o) \
		$(LIBRARY)(showdate.o) \
		$(LIBRARY)(working.o) 

$(LIBRARY)(fits.o): $(CURSES_H)/curses.h
$(LIBRARY)(fits.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(fits.o): $(HEADER1)/wish.h
$(LIBRARY)(fits.o): fits.c

$(LIBRARY)(hide.o): $(CURSES_H)/curses.h
$(LIBRARY)(hide.o): $(HEADER1)/color_pair.h
$(LIBRARY)(hide.o): $(HEADER1)/vt.h
$(LIBRARY)(hide.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(hide.o): $(HEADER1)/wish.h
$(LIBRARY)(hide.o): hide.c

$(LIBRARY)(highlight.o): $(CURSES_H)/curses.h
$(LIBRARY)(highlight.o): $(HEADER1)/color_pair.h
$(LIBRARY)(highlight.o): highlight.c

$(LIBRARY)(indicator.o): $(CURSES_H)/curses.h
$(LIBRARY)(indicator.o): $(HEADER1)/vt.h
$(LIBRARY)(indicator.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(indicator.o): $(HEADER1)/wish.h
$(LIBRARY)(indicator.o): indicator.c

$(LIBRARY)(lp.o): $(CURSES_H)/curses.h
$(LIBRARY)(lp.o): lp.c

$(LIBRARY)(makebox.o): $(CURSES_H)/curses.h
$(LIBRARY)(makebox.o): $(HEADER1)/attrs.h
$(LIBRARY)(makebox.o): $(HEADER1)/vt.h
$(LIBRARY)(makebox.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(makebox.o): $(HEADER1)/wish.h
$(LIBRARY)(makebox.o): makebox.c

$(LIBRARY)(message.o): $(CURSES_H)/curses.h
$(LIBRARY)(message.o): $(HEADER1)/message.h
$(LIBRARY)(message.o): $(HEADER1)/vt.h
$(LIBRARY)(message.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(message.o): $(HEADER1)/wish.h
$(LIBRARY)(message.o): message.c

$(LIBRARY)(move.o): $(CURSES_H)/curses.h
$(LIBRARY)(move.o): $(HEADER1)/vt.h
$(LIBRARY)(move.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(move.o): $(HEADER1)/wish.h
$(LIBRARY)(move.o): move.c

$(LIBRARY)(offscreen.o): $(CURSES_H)/curses.h
$(LIBRARY)(offscreen.o): $(HEADER1)/wish.h
$(LIBRARY)(offscreen.o): offscreen.c

$(LIBRARY)(physical.o): $(CURSES_H)/curses.h
$(LIBRARY)(physical.o): $(HEADER1)/actrec.h
$(LIBRARY)(physical.o): $(HEADER1)/message.h
$(LIBRARY)(physical.o): $(HEADER1)/moremacros.h
$(LIBRARY)(physical.o): $(HEADER1)/token.h
$(LIBRARY)(physical.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(physical.o): $(HEADER1)/vt.h
$(LIBRARY)(physical.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(physical.o): $(HEADER1)/wish.h
$(LIBRARY)(physical.o): physical.c

$(LIBRARY)(redraw.o): $(CURSES_H)/curses.h
$(LIBRARY)(redraw.o): $(HEADER1)/vt.h
$(LIBRARY)(redraw.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(redraw.o): $(HEADER1)/wish.h
$(LIBRARY)(redraw.o): redraw.c

$(LIBRARY)(showdate.o): $(CURSES_H)/curses.h
$(LIBRARY)(showdate.o): $(HEADER1)/ctl.h
$(LIBRARY)(showdate.o): $(HEADER1)/vt.h
$(LIBRARY)(showdate.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(showdate.o): $(HEADER1)/wish.h
$(LIBRARY)(showdate.o): showdate.c

$(LIBRARY)(showmail.o): $(CURSES_H)/curses.h
$(LIBRARY)(showmail.o): $(HEADER1)/vt.h
$(LIBRARY)(showmail.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(showmail.o): $(HEADER1)/wish.h
$(LIBRARY)(showmail.o): showmail.c

$(LIBRARY)(system.o): $(CURSES_H)/curses.h
$(LIBRARY)(system.o): $(HEADER1)/wish.h
$(LIBRARY)(system.o): system.c

$(LIBRARY)(vclose.o): $(CURSES_H)/curses.h
$(LIBRARY)(vclose.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(vclose.o): $(HEADER1)/vt.h
$(LIBRARY)(vclose.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(vclose.o): $(HEADER1)/wish.h
$(LIBRARY)(vclose.o): vclose.c

$(LIBRARY)(vcolor.o): $(CURSES_H)/curses.h
$(LIBRARY)(vcolor.o): $(HEADER1)/color_pair.h
$(LIBRARY)(vcolor.o): $(HEADER1)/moremacros.h
$(LIBRARY)(vcolor.o): $(HEADER1)/vt.h
$(LIBRARY)(vcolor.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(vcolor.o): $(HEADER1)/wish.h
$(LIBRARY)(vcolor.o): vcolor.c

$(LIBRARY)(vcreate.o): $(CURSES_H)/curses.h
$(LIBRARY)(vcreate.o): $(HEADER1)/color_pair.h
$(LIBRARY)(vcreate.o): $(HEADER1)/moremacros.h
$(LIBRARY)(vcreate.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(vcreate.o): $(HEADER1)/vt.h
$(LIBRARY)(vcreate.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(vcreate.o): $(HEADER1)/wish.h
$(LIBRARY)(vcreate.o): vcreate.c

$(LIBRARY)(vctl.o): $(CURSES_H)/curses.h
$(LIBRARY)(vctl.o): $(HEADER1)/attrs.h
$(LIBRARY)(vctl.o): $(HEADER1)/color_pair.h
$(LIBRARY)(vctl.o): $(HEADER1)/ctl.h
$(LIBRARY)(vctl.o): $(HEADER1)/vt.h
$(LIBRARY)(vctl.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(vctl.o): $(HEADER1)/wish.h
$(LIBRARY)(vctl.o): vctl.c

$(LIBRARY)(vcurrent.o): $(CURSES_H)/curses.h
$(LIBRARY)(vcurrent.o): $(HEADER1)/color_pair.h
$(LIBRARY)(vcurrent.o): $(HEADER1)/vt.h
$(LIBRARY)(vcurrent.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(vcurrent.o): $(HEADER1)/wish.h
$(LIBRARY)(vcurrent.o): vcurrent.c

$(LIBRARY)(vdebug.o): $(CURSES_H)/curses.h
$(LIBRARY)(vdebug.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(vdebug.o): $(HEADER1)/vt.h
$(LIBRARY)(vdebug.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(vdebug.o): $(HEADER1)/wish.h
$(LIBRARY)(vdebug.o): vdebug.c

$(LIBRARY)(vflush.o): $(CURSES_H)/curses.h
$(LIBRARY)(vflush.o): $(HEADER1)/attrs.h
$(LIBRARY)(vflush.o): $(HEADER1)/color_pair.h
$(LIBRARY)(vflush.o): $(HEADER1)/vt.h
$(LIBRARY)(vflush.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(vflush.o): $(HEADER1)/wish.h
$(LIBRARY)(vflush.o): vflush.c

$(LIBRARY)(vfork.o): $(CURSES_H)/curses.h
$(LIBRARY)(vfork.o): $(HEADER1)/wish.h
$(LIBRARY)(vfork.o): vfork.c

$(LIBRARY)(vinit.o): $(CURSES_H)/curses.h
$(LIBRARY)(vinit.o): $(HEADER1)/attrs.h
$(LIBRARY)(vinit.o): $(HEADER1)/ctl.h
$(LIBRARY)(vinit.o): $(HEADER1)/token.h
$(LIBRARY)(vinit.o): $(HEADER1)/var_arrays.h
$(LIBRARY)(vinit.o): $(HEADER1)/vt.h
$(LIBRARY)(vinit.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(vinit.o): $(HEADER1)/wish.h
$(LIBRARY)(vinit.o): vinit.c

$(LIBRARY)(vmark.o): $(CURSES_H)/curses.h
$(LIBRARY)(vmark.o): $(HEADER1)/vt.h
$(LIBRARY)(vmark.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(vmark.o): $(HEADER1)/wish.h
$(LIBRARY)(vmark.o): vmark.c

$(LIBRARY)(vreshape.o): $(CURSES_H)/curses.h
$(LIBRARY)(vreshape.o): $(HEADER1)/color_pair.h
$(LIBRARY)(vreshape.o): $(HEADER1)/vt.h
$(LIBRARY)(vreshape.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(vreshape.o): $(HEADER1)/wish.h
$(LIBRARY)(vreshape.o): vreshape.c

$(LIBRARY)(wclrwin.o): $(CURSES_H)/curses.h
$(LIBRARY)(wclrwin.o): $(HEADER1)/attrs.h
$(LIBRARY)(wclrwin.o): $(HEADER1)/vt.h
$(LIBRARY)(wclrwin.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(wclrwin.o): $(HEADER1)/wish.h
$(LIBRARY)(wclrwin.o): wclrwin.c

$(LIBRARY)(wdelchar.o): $(CURSES_H)/curses.h
$(LIBRARY)(wdelchar.o): $(HEADER1)/vt.h
$(LIBRARY)(wdelchar.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(wdelchar.o): $(HEADER1)/wish.h
$(LIBRARY)(wdelchar.o): wdelchar.c

$(LIBRARY)(wgetchar.o): $(CURSES_H)/curses.h
$(LIBRARY)(wgetchar.o): $(HEADER1)/token.h
$(LIBRARY)(wgetchar.o): $(HEADER1)/vt.h
$(LIBRARY)(wgetchar.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(wgetchar.o): $(HEADER1)/wish.h
$(LIBRARY)(wgetchar.o): wgetchar.c

$(LIBRARY)(wgo.o): $(CURSES_H)/curses.h
$(LIBRARY)(wgo.o): $(HEADER1)/vt.h
$(LIBRARY)(wgo.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(wgo.o): $(HEADER1)/wish.h
$(LIBRARY)(wgo.o): wgo.c

$(LIBRARY)(winschar.o): $(CURSES_H)/curses.h
$(LIBRARY)(winschar.o): $(HEADER1)/vt.h
$(LIBRARY)(winschar.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(winschar.o): $(HEADER1)/wish.h
$(LIBRARY)(winschar.o): winschar.c

$(LIBRARY)(working.o): $(CURSES_H)/curses.h
$(LIBRARY)(working.o): $(HEADER1)/vt.h
$(LIBRARY)(working.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(working.o): $(HEADER1)/wish.h
$(LIBRARY)(working.o): working.c

$(LIBRARY)(wprintf.o): $(CURSES_H)/curses.h
$(LIBRARY)(wprintf.o): $(HEADER1)/vt.h
$(LIBRARY)(wprintf.o): $(HEADER1)/wish.h
$(LIBRARY)(wprintf.o): wprintf.c

$(LIBRARY)(wputchar.o): $(CURSES_H)/curses.h
$(LIBRARY)(wputchar.o): $(HEADER1)/vt.h
$(LIBRARY)(wputchar.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(wputchar.o): $(HEADER1)/wish.h
$(LIBRARY)(wputchar.o): wputchar.c

$(LIBRARY)(wputs.o): $(CURSES_H)/curses.h
$(LIBRARY)(wputs.o): $(HEADER1)/attrs.h
$(LIBRARY)(wputs.o): $(HEADER1)/vt.h
$(LIBRARY)(wputs.o): $(HEADER1)/wish.h
$(LIBRARY)(wputs.o): wputs.c

$(LIBRARY)(wreadchar.o): $(CURSES_H)/curses.h
$(LIBRARY)(wreadchar.o): $(HEADER1)/vt.h
$(LIBRARY)(wreadchar.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(wreadchar.o): $(HEADER1)/wish.h
$(LIBRARY)(wreadchar.o): wreadchar.c

$(LIBRARY)(wscrollwin.o): $(CURSES_H)/curses.h
$(LIBRARY)(wscrollwin.o): $(HEADER1)/attrs.h
$(LIBRARY)(wscrollwin.o): $(HEADER1)/vt.h
$(LIBRARY)(wscrollwin.o): $(HEADER1)/vtdefs.h
$(LIBRARY)(wscrollwin.o): $(HEADER1)/wish.h
$(LIBRARY)(wscrollwin.o): wscrollwin.c

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
