#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)mouse:cmd/Makefile	1.3.1.1"

LDFLAGS=-s

all:	mousemgr mouseadmin

install package:	all 
	-ls mousemgr mouseadmin | cpio -pduv ../pkg

clean: $(FRC)
	rm -f *.o

clobber: clean
	rm -f mousemgr mouseadmin

mousemgr:	mousemgr.o $(FRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o mousemgr mousemgr.o $(LDLIBS)

mouseadmin:	mouseadmin.o $(FRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o mouseadmin mouseadmin.o $(LDLIBS) -lcurses

.c.o:	$(FRC)
	$(CC) $(CFLAGS) -c $*.c

FRC:
