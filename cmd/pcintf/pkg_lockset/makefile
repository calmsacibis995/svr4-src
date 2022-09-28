#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)pcintf:pkg_lockset/makefile	1.1"
#! /bin/make -f
#@(#)makefile	1.5	18:10:43	10/12/89 
#
# Locus Bridge Products Group - LOCKSET Package Makefile
#

LIB_NAME=	liblset.a
CFPLUS=		$(CF) -I.
CFLAGS=		$(DFLAGS) $(CFPLUS) -O
LINTFLAGS=	$(CFPLUS) -u -v -x

LIB_OBJ=	\
		lockset.o \
		lset_estr.o \
		lsetclr.o \
		lsetcount.o \
		lsetnew.o \
		lsetrmv.o \
		lsetset.o \
		lsetuse.o \
		lsetunuse.o

LIB_LINTS=	\
		lockset.ln \
		lset_estr.ln \
		lsetclr.ln \
		lsetcount.ln \
		lsetnew.ln \
		lsetrmv.ln \
		lsetset.ln \
		lsetuse.ln \
		lsetunuse.ln

# default target: library

all:		$(LIB_NAME)
	@echo "\nLocus Bridge Products Group - LOCKSET Package is done.\n"

# don't let the library be deleted when make gets killed

.PRECIOUS:	$(LIB_NAME)
.SUFFIXES:	.ln

$(LIB_NAME):	$(LIB_OBJ)
	ar r $(LIB_NAME) $(LIB_OBJ)
	sh -c "ar ts $(LIB_NAME) || ranlib $(LIB_NAME)"
	@echo "$(LIB_NAME) updated with:"; echo "\t$?"
	ar t $(LIB_NAME)

# clean out expected binaries.

clean:
	rm -f $(LIB_NAME) *.o

# lint stuff

.c.ln:
	@lint $(LINTFLAGS) -c $<

lint:		$(LIB_LINTS)
