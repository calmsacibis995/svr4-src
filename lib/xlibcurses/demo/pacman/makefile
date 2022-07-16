#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)curses:demo/pacman/makefile	1.3"
CC =	cc
DFLAGS =
CFLAGS =	-O
LDFLAGS =
CFILES =	pacman.c monster.c util.c movie.c
OFILES =	pacman.o monster.o util.o movie.o

pacman:	$(OFILES)
	$(CC) $(LDFLAGS) -o pacman $(OFILES) -lcurses

pacman.o:	pacman.c pacdefs.h
	$(CC) -c $(CFLAGS) $(DFLAGS) pacman.c

monster.o:	monster.c pacdefs.h
	$(CC) -c $(CFLAGS) $(DFLAGS) monster.c

util.o:	util.c pacdefs.h
	$(CC) -c $(CFLAGS) $(DFLAGS) util.c

movie.o:	movie.c pacdefs.h
	$(CC) -c $(CFLAGS) $(DFLAGS) movie.c


install:	pacman
	cp pacman /usr/games/pacman

strip:	pacman
	strip pacman

shrink:
	-rm -f *.o

clean:	shrink
	-rm -f pacman errs core a.out

lint:	$(CFILES)
	lint -pc $(CFILES)

list:	$(CFILES) pacdefs.h makefile
	oprl -x makefile $(CFILES) pacdefs.h
	oprl -x -C $(CFILES)
	touch list
