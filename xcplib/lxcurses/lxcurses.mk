#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1987, 1988 Microsoft Corporation
#	  All Rights Reserved

#	This Module contains Proprietary Information of Microsoft
#	Corporation and should be treated as Confidential.	  

#ident	"@(#)xcplxcurses:lxcurses.mk	1.1"

#
#	@(#) lxcurses.mk 1.1 90/03/30 lxcurses:lxcurses.mk
#
LGTXTNM	= _LIB_TEXT

INS=install
LIBCUR	= libxcurses.a
INCS	= xcurses.h ext.h

LIBOBJS	= \
	$(LIBCUR)(box.o)	$(LIBCUR)(clear.o)	$(LIBCUR)(clrtobot.o) \
	$(LIBCUR)(delch.o)	$(LIBCUR)(deleteln.o)	$(LIBCUR)(endwin.o) \
	$(LIBCUR)(initscr.o)	$(LIBCUR)(insch.o)	$(LIBCUR)(cr_tty.o) \
	$(LIBCUR)(delwin.o)	$(LIBCUR)(insertln.o)	$(LIBCUR)(longname.o) \
	$(LIBCUR)(move.o)	$(LIBCUR)(mvprintw.o)	$(LIBCUR)(mvscanw.o) \
	$(LIBCUR)(mvwin.o)	$(LIBCUR)(newwin.o)	$(LIBCUR)(overlay.o) \
	$(LIBCUR)(overwrite.o)	$(LIBCUR)(printw.o)	$(LIBCUR)(addstr.o) \
	$(LIBCUR)(scanw.o)	$(LIBCUR)(getstr.o)	$(LIBCUR)(getch.o) \
	$(LIBCUR)(addch.o)	$(LIBCUR)(clrtoeol.o)	$(LIBCUR)(standout.o) \
	$(LIBCUR)(unctrl.o)	$(LIBCUR)(cr_put.o)	$(LIBCUR)(refresh.o) \
	$(LIBCUR)(erase.o)	$(LIBCUR)(scroll.o)	$(LIBCUR)(curses.o) \
	$(LIBCUR)(touchwin.o)	$(LIBCUR)(tstp.o)	$(LIBCUR)(save_mode.o)

.PRECIOUS:	$(LIBCUR)

###
# standard targets
#
all:	$(LIBOBJS)

install:	all
	$(INS) -f $(ROOT)/usr/lib -m 644 -u bin -g bin $(LIBCUR) 
	$(INS) -f $(ROOT)/usr/include -m 644 -u root -g sys xcurses.h
clean:
	rm -f $(LIBCUR)

clobber: clean


###
# extra targets
#
$(LIBOBJS):	$(INCS)
