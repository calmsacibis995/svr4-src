#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)xcplibx:libx/libx.mk	1.1"

#	Copyright (c) 1987, 1988 Microsoft Corporation
#	  All Rights Reserved

#	This Module contains Proprietary Information of Microsoft
#	Corporation and should be treated as Confidential.	  

#
# makefile for libx
#
#
# The variable PROF is null by default, causing both the standard XENIX library
# and a profiled library to be maintained.  If profiled object is not 
# desired, the reassignment PROF=@# should appear in the make command line.
#
# The variable IGN may be set to -i by the assignment IGN=-i in order to
# allow a make to complete even if there are compile errors in individual
# modules.
#
# See also the comments in the lower-level machine-dependent makefiles.
#

VARIANT=
SGS=
CFLAGS=-O -DMERGE
PCFLAGS=
PFX=
CC=$(PFX)cc
AR=ar
LORDER=$(PFX)lorder
ROOT=
LIB=$(ROOT)/usr/lib
LIBP=$(ROOT)/usr/lib/libp
DONE=
MODEL=
PROF=
NONPROF=
DEFLIST=

all:
	if i386 ; then $(MAKE) -e -f libx.mk specific MACHINE=i386 ; fi

specific:
	#
	# compile portable library modules
	cd port; $(MAKE) -e CC=$(CC) CFLAGS="$(CFLAGS)"
	#
	# compile machine-dependent library modules
	cd $(MACHINE); $(MAKE) -e CC=$(CC) CFLAGS="$(CFLAGS)"
	#
	# place portable modules in "object" directory, then overlay
	# 	the machine-dependent modules.
	-rm -rf object
	mkdir object
	-cp port/*/*.o object
#	cp port/*/[a-l]*.o object
#	cp port/*/[!a-l]*.o object
	-$(PROF)cp port/*/*.p object
#	$(PROF)cp port/*/[a-l]*.p object
#	$(PROF)cp port/*/[!a-l]*.p object
	cp $(MACHINE)/*/*.o object
#	cp $(MACHINE)/*/[a-l]*.o object
#	cp $(MACHINE)/*/[!a-l]*.o object
	$(PROF)cp $(MACHINE)/*/*.p object
#	$(PROF)cp $(MACHINE)/*/[a-l]*.p object
#	$(PROF)cp $(MACHINE)/*/[!a-l]*.p object
	#
	# delete temporary libraries
	-rm -f lib.libx
	$(PROF)-rm -f libp.libx
	#
	# build archive out of the remaining modules.
	cd object; $(MAKE) -e -f ../$(MACHINE)/makefile archive \
		AR=$(AR)  STRIP=$(STRIP) LORDER=$(LORDER) PROF=$(PROF)
	-rm -rf object
	#
	$(DONE)

move:
	#
	# move the library or libraries into the correct directory
	cp lib.libx $(LIB)/lib$(VARIANT)x.a ; rm -f lib.libx
	$(PROF)if [ ! -d $(LIBP) ]; then \
	$(PROF)	mkdir $(LIBP); \
	$(PROF)fi
	$(PROF)cp libp.libx $(LIBP)/lib$(VARIANT)x.a ; rm -f libp.libx

install: all move

clean:
	#
	# remove intermediate files except object modules and temp library
	-rm -rf lib*.contents obj*
	cd port ;  make clean
	if i386 ; then cd i386 ; $(MAKE) clean ; fi

clobber:
	#
	# remove intermediate files
	-rm -rf *.o lib*.libx lib*.contents obj*
	cd port ;  make clobber
	if i386 ;  then cd i386 ; $(MAKE) clobber ; fi
