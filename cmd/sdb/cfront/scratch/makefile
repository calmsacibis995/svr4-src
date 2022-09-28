#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sdb:cfront/scratch/makefile	1.5"
LIBFILES=_ctor..o _delete..o _dtor..o _handler..o \
_main..o _new..o _vec..o dtors..o exit..o

CC=cc
AR=ar
NM=nm

SRCFILES=alloc..o dcl..o dcl2..o dcl3..o del..o error..o \
expand..o expr..o expr2..o lalex..o lex..o \
main..o norm..o norm2..o print..o repr..o \
simpl..o size..o table..o typ..o typ2..o \
y.tab..o

BSD=

all:	munch xcfront libC.a
	mv xcfront libC.a munch ..

libC.a:	$(LIBFILES)
	$(AR) rv libC.a $(LIBFILES)
	if test "$(BSD)"; then ranlib libC.a; fi

munch:	mnch/munch..c libC.a
	$(CC) -o munch mnch/munch..c libC.a

xcfront: libC.a $(SRCFILES)
	$(CC) -o xcfront $(SRCFILES) libC.a
	$(NM) xcfront | ./munch >__ctdt..c
	$(CC) -c __ctdt..c
	$(CC) -o xcfront __ctdt..o $(SRCFILES) libC.a

_ctor..o:	lib/_ctor..c
	$(CC) -c lib/_ctor..c

_delete..o:	lib/_delete..c
	$(CC) -c lib/_delete..c

_dtor..o:	lib/_dtor..c
	$(CC) -c lib/_dtor..c

_handler..o:	lib/_handler..c
	$(CC) -c lib/_handler..c

_main..o:	lib/_main..c
	$(CC) -c lib/_main..c

_new..o:	lib/_new..c
	$(CC) -c lib/_new..c

_vec..o:	lib/_vec..c
	$(CC) -c lib/_vec..c

dtors..o:	lib/dtors..c
	$(CC) -c lib/dtors..c

exit..o:	lib/exit..c
	$(CC) -c lib/exit..c

alloc..o:	src/alloc..c
	$(CC) -c src/alloc..c

dcl..o:	src/dcl..c
	$(CC) -c src/dcl..c

dcl2..o:	src/dcl2..c
	$(CC) -c src/dcl2..c

dcl3..o:	src/dcl3..c
	$(CC) -c src/dcl3..c

del..o:	src/del..c
	$(CC) -c src/del..c

error..o:	src/error..c
	$(CC) -c src/error..c

expand..o:	src/expand..c
	$(CC) -c src/expand..c

expr..o:	src/expr..c
	$(CC) -c src/expr..c

expr2..o:	src/expr2..c
	$(CC) -c src/expr2..c

lalex..o:	src/lalex..c
	$(CC) -c src/lalex..c

lex..o:	src/lex..c
	$(CC) -c src/lex..c

main..o:	src/main..c
	$(CC) -c src/main..c

norm..o:	src/norm..c
	$(CC) -c src/norm..c

norm2..o:	src/norm2..c
	$(CC) -c src/norm2..c

print..o:	src/print..c
	$(CC) -c src/print..c

repr..o:	src/repr..c
	$(CC) -c src/repr..c

simpl..o:	src/simpl..c
	$(CC) -c src/simpl..c

size..o:	src/size..c
	$(CC) -c src/size..c

table..o:	src/table..c
	$(CC) -c src/table..c

typ..o:	src/typ..c
	$(CC) -c src/typ..c

typ2..o:	src/typ2..c
	$(CC) -c src/typ2..c

y.tab..o:	src/y.tab..c
	$(CC) -c src/y.tab..c
