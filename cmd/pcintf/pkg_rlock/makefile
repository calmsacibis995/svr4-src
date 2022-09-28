#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)pcintf:pkg_rlock/makefile	1.1"

#! /bin/make -f
#@(#)makefile	1.5	17:58:58	10/12/89 
#
# Locus Bridge Products Group - RLOCK Package Makefile
#

PROGS=		rlockshm
LIB_NAME=	librlock.a

CFPLUS=		$(CF) -I. -I../pkg_lockset
CFLAGS=		$(DFLAGS) $(CFPLUS) -O
LINTFLAGS=	$(CFPLUS) -u -v -x
LDFLAGS=	$(LF)
LIBFLAGS=	$(LIBF) ./librlock.a ../pkg_lockset/liblset.a

RLOCKSHM_OBJ=	\
		rlockshm.o

LIB_OBJ=	\
		multi_init.o \
		multi_lock.o \
		multi_open.o \
		rlock_estr.o \
		rlock_init.o \
		rlock_prim.o \
		shm_lock.o \
		shm_open.o \
		state.o

LIB_LINTS=	\
		multi_init.ln \
		multi_lock.ln \
		multi_open.ln \
		rlock_estr.ln \
		rlock_init.ln \
		rlock_prim.ln \
		shm_lock.ln \
		shm_open.ln \
		state.ln

RLOCKSHM_LINTS=	\
		rlockshm.ln

# default target: library

all:		$(LIB_NAME) $(PROGS)
	@echo "\nLocus Bridge Products Group - RLOCK Package is done.\n"

# don't let the library be deleted when make gets killed

.PRECIOUS:	$(LIB_NAME)
.SUFFIXES:	.ln

$(LIB_NAME):	$(LIB_OBJ)
	ar r $(LIB_NAME) $(LIB_OBJ)
	sh -c "ar ts $(LIB_NAME) || ranlib $(LIB_NAME)"
	@echo "$(LIB_NAME) updated with:"; echo "\t$?"
	ar t $(LIB_NAME)

# RLOCK shared memory manipulation

rlockshm:	$(RLOCKSHM_OBJ) $(LIB_NAME)
	-$(CC) $(LDFLAGS) -o $@ $(RLOCKSHM_OBJ) $(LIBFLAGS)
	strip $@

# clean out expected binaries.

clean:
	rm -f $(LIB_NAME) $(PROGS) *.o *.ln

# lint stuff

.c.ln:
	@lint $(LINTFLAGS) -c $<

lint:		$(LIB_LINTS) \
		$(RLOCKSHM_LINTS)
