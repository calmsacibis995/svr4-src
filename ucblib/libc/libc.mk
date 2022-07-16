#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)ucblibc:libc.mk	1.6.3.1"
#
# makefile for libc
#
#
CFLAGS=-O
PFX=
CC=$(PFX)cc
AR=$(PFX)ar
LORDER=$(PFX)lorder
TSORT=$(PFX)tsort
LIB=$(ROOT)/usr/ucblib
DONE=
PROF=
NONPROF=
DEFLIST=
OWN=bin
GRP=bin

all: specific

specific:
	#
	# compile portable library modules
	cd port; make -e CC=$(CC) $(INS)
	cd i386; make -e
	#
	# place portable modules in "object" directory, then overlay
	# 	the machine-dependent modules.
	-rm -rf object
	mkdir object
	cp port/*/*.o object
	cp i386/*/*.o object
	#
	# delete temporary libraries
	-rm -f lib.libucb
	#
	# build archive out of the remaining modules.
	cd object; make -e -f ../i386/makefile archive \
		AR=$(AR)  LORDER=$(LORDER) PROF=$(PROF) MAC=$(MAC)
	-rm -rf object
	#
	$(DONE)

install: all
	#
	# move the library or libraries into the correct directory
	mv lib.libucb lib$(VARIANT)ucb.a ; \
	install -m 644 -u $(OWN) -g $(GRP) -f $(LIB) lib$(VARIANT)ucb.a ; \
	rm -f lib$(VARIANT)ucb.a

clean:
	#
	# remove intermediate files except object modules and temp library
	-rm -rf lib*.contents obj*
	cd port ;  make clean
	cd i386; make clean

clobber:
	#
	# remove intermediate files
	-rm -rf *.o lib*.libucb lib*.contents obj*
	cd port; make clobber
	cd i386; make clobber
