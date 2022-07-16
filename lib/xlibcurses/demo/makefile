#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)curses:demo/makefile	1.3"
LFLAGS=	-lcurses
CFLAGS= -O -s

all: aliens rain menu worms

aliens:	aliens.c
	$(CC) $(CFLAGS) aliens.c -o aliens $(LFLAGS)

rain: rain.c
	$(CC) $(CFLAGS) rain.c -o rain $(LFLAGS)

menu: menu.c
	$(CC) $(CFLAGS) menu.c -o menu $(LFLAGS)

worms: worms.c
	$(CC) $(CFLAGS) worms.c -o worms $(LFLAGS)
