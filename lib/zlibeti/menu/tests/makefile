#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

PGMS=test1 test2 test3 test4 test5 test6 test7 test8\
     test9 test10 test11 test12 test13 test14 test15

all:		$(PGMS)

.c.o:
		cc -I.. -gc $<

$(PGMS):	$$@.o ../libmenu.a virtual.o
		cc $@.o virtual.o ../../panels/sti.a ../libmenu.a libcurses.a -go $@
