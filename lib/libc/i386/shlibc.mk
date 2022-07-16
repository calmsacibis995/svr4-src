#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)shlibc:i386/shlibc.mk	1.3.6.3"
#
#
# makefile for shared libc
# 
#
# there will be no profiled objects with shared libraries.
#

SGS=
CFLAGS=-O
PCFLAGS=
PFX=
CC=$(PFX)cc
AR=ar
LORDER=$(PFX)lorder
ROOT=
LIB=$(ROOT)/lib
SHLIB=$(ROOT)/shlib
DEFLIST=
SDEFLIST=
ISANSI=
PICDEF=pic.def

all: 
	make -e -f shlibc.mk shared \
		PROF=@# MACHINE=i386 DEFLIST=-Di386 SDEFLIST=-DDSHLIB \
		CFLAGS="$(CFLAGS) -Kpic"

shared:
	#
	# compile portable library modules
	cd ../port; make -e CC=$(CC)
	#
	# compile machine-dependent library modules
	make -e CC=$(CC) PICDEF=$(PICDEF) ISANSI=$(ISANSI)
	#
	# place portable modules in "object" directory, then overlay
	#	the machine-dependent modules.
	-rm -rf ../object
	-mkdir ../object
	cd ..; cp port/*/[_a-m]*.o object
	cd ..; cp port/*/[n-z]*.o object
	cd ..; cp $(MACHINE)/*/*.o object
	#
	# rm files with duplicate definitions from shared library
	cd ../object; rm -f *crt?.o fakcu.o fpstart0.o values-Xa.o values-Xc.o
	#
	# delete old library
	-rm -f libc.so
	#
	# create dynamic shared library
	# cd ../object; $(CC) -o ../libc.so -dy -G -ztext *.o
	cd ../object; $(CC) -o ../libc.so -dy -G *.o
	-rm -rf ../object
move:
	# Move the shared library into the correct directory
	#
	cp ../libc.so $(LIB)/libc.so

install:all move

clean:
	#
	# remove intermediate files
	cd ..; rm -rf *.o X* *.x objlist 
	cd ../port ;  make clean
	make clean
	#
clobber:
	#
	# 
	#
	-rm -rf ../libc.so
	cd ..; rm -rf *.o lib*.libc obj* X* *.x
	cd ../port; make clobber
	make clobber
	# done
	#
