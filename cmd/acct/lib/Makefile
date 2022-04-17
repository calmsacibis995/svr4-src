#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)acct:lib/Makefile	1.8.2.2"
TESTDIR = .
LIB = a.a
CFLAGS = -O
INCDIR = ..
OPTFLAGS = -D_LTYPES
FRC =

FILES= \
	$(LIB)(pnpsplit.o) \
	$(LIB)(copyn.o) \
	$(LIB)(devtolin.o) \
	$(LIB)(expand.o) \
	$(LIB)(lintodev.o) \
	$(LIB)(namtouid.o) \
	$(LIB)(tmless.o) \
	$(LIB)(tmsecs.o) \
	$(LIB)(uidtonam.o) \
	$(LIB)(substr.o)

all:	$(LIB) $(FRC)

$(LIB):	$(FILES)
	@echo lib $(LIB) is now up to date

clean:
	-rm -f *.o

clobber:	clean
	-rm -f $(LIB)

FRC:

.c.a:
	$(CC) $(CFLAGS) $(OPTFLAGS) -I$(INCDIR) -c $<
	$(AR) rv $@ $*.o
	rm -f $*.o

