#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ucblibcurses:libcurses.mk	1.1.1.1"

#		PROPRIETARY NOTICE (Combined)
#
#This source code is unpublished proprietary information
#constituting, or derived under license from AT&T's UNIX(r) System V.
#In addition, portions of such source code were derived from Berkeley
#4.3 BSD under license from the Regents of the University of
#California.
#
#
#
#		Copyright Notice 
#
#Notice of copyright on this source code product does not indicate 
#publication.
#
#	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
#	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
#	          All rights reserved.


#
# makefile for libcurses.a
#
#

CC=$(PFX)cc
CFLAGS= -O
AR=$(PFX)ar
LORDER=$(PFX)lorder
TSORT=$(PFX)tsort
PROF=
NONPROF=
INC=$(ROOT)/usr/include
INC1=$(ROOT)/usr/ucbinclude
INCSYS=$(ROOT)/usr/include
INCSYS1=$(ROOT)/usr/ucbinclude
DEFLIST=
SDEFLIST=
LIB=$(ROOT)/usr/ucblib
INS=install

OBJECTS = addch.o addstr.o box.o clear.o clrtobot.o clrtoeol.o cr_put.o \
	cr_tty.o curses.o delch.o deleteln.o delwin.o endwin.o erase.o \
	fullname.o getch.o getstr.o id_subwins.o idlok.o initscr.o insch.o \
	insertln.o longname.o move.o mvprintw.o mvscanw.o mvwin.o newwin.o \
	overlay.o overwrite.o printw.o putchar.o refresh.o scanw.o \
	scroll.o standout.o toucholap.o touchwin.o tstp.o unctrl.o

SOURCES = addch.c addstr.c box.c clear.c clrtobot.c clrtoeol.c cr_put.c \
	cr_tty.c curses.c delch.c deleteln.c delwin.c endwin.c erase.c \
	fullname.c getch.c getstr.c id_subwins.c idlok.c initscr.c insch.c \
	insertln.c longname.c move.c mvprintw.c mvscanw.c mvwin.c newwin.c \
	overlay.c overwrite.c printw.c putchar.c refresh.c scanw.c \
	scroll.c standout.c toucholap.c touchwin.c tstp.c unctrl.c


ALL:		 $(OBJECTS) libcurses.a

addch.o: curses.ext curses.h
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c addch.c

addstr.o: curses.ext curses.h
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c addstr.c

box.o: curses.ext curses.h
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c box.c

clear.o: curses.ext curses.h
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c clear.c

clrtobot.o: curses.ext curses.h
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c clrtobot.c

clrtoeol.o: curses.ext curses.h
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c clrtoeol.c

cr_put.o: curses.ext curses.h
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c cr_put.c

cr_tty.o: curses.ext curses.h
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c cr_tty.c

curses.o: curses.h
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c curses.c

delch.o: curses.ext curses.h
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c delch.c

deleteln.o: curses.ext curses.h 
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c deleteln.c 

delwin.o: curses.ext curses.h 
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c delwin.c 

endwin.o: curses.ext curses.h 
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c endwin.c

erase.o: curses.ext curses.h
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c erase.c

fullname.o: curses.ext curses.h
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c fullname.c

getch.o: curses.ext curses.h
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c getch.c

getstr.o: curses.ext curses.h
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c getstr.c

id_subwins.o: curses.ext curses.h
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c id_subwins.c

idlok.o: curses.ext curses.h 
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c idlok.c

initscr.o: curses.ext curses.h $(INC1)/signal.h
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c initscr.c

insch.o: curses.ext curses.h
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c insch.c

insertln.o: curses.ext curses.h
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c insertln.c

longname.o:
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c longname.c

move.o: curses.ext curses.h
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c move.c

mvprintw.o: curses.ext curses.h $(INC)/varargs.h
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c mvprintw.c

mvscanw.o: curses.ext curses.h $(INC)/varargs.h
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c mvscanw.c

mvwin.o: curses.ext curses.h
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c mvwin.c

newwin.o: curses.ext curses.h
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c newwin.c

overlay.o: curses.ext curses.h $(INC)/ctype.h
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c overlay.c

overwrite.o: curses.ext curses.h $(INC)/ctype.h
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c overwrite.c

printw.o: curses.ext curses.h $(INC)/varargs.h
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c printw.c

putchar.o: curses.ext curses.h 
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c putchar.c

scanw.o: curses.ext curses.h $(INC)/varargs.h
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c scanw.c

scroll.o: curses.ext curses.h 
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c scroll.c

stdout.o: curses.ext curses.h 
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c stdout.c

toucholap.o: curses.ext curses.h 
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c toucholap.c

touchwin.o: curses.ext curses.h 
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c touchwin.c

tstp.o: curses.ext curses.h $(INC1)/signal.h
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c tstp.c

unctrl.o: curses.ext curses.h 
		$(CC)  $(CFLAGS) -I. -I$(INC1) -c unctrl.c

GLOBALINCS = ./curses.ext \
	./curses.h \
	$(INC)/ctype.h \
	$(INC1)/signal.h \
	$(INC)/varargs.h 

libcurses.a: 
	$(AR) q libcurses.a `$(LORDER) *.o | $(TSORT)`

install: ALL
	$(INS) -f $(LIB) -m 644 libcurses.a

clean:
	rm -f $(OBJECTS)

clobber:
	rm -f $(OBJECTS) libcurses.a
