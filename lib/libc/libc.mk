#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libc:libc.mk	1.27.3.4"
#
# makefile for libc
#
#
# The variable PROF is null by default, causing both the standard C library
# and a profiled library to be maintained.  If profiled object is not 
# desired, the reassignment PROF=@# should appear in the make command line.
#
# The variable IGN may be set to -i by the assignment IGN=-i in order to
# allow a make to complete even if there are compile errors in individual
# modules.
#
# See also the comments in the lower-level machine-dependent makefiles.
#

MAC=
VARIANT=
SGS=
CFLAGS=-O
PCFLAGS=
PFX=
CC=$(PFX)cc
AR=ar
LORDER=$(PFX)lorder
MAKE=make
ROOT=
CCSLIB=$(ROOT)/usr/ccs/lib
USRLIB=$(ROOT)/usr/lib
LIBP=$(CCSLIB)/libp
ABILIB=$(CCSLIB)/minabi
ABILIBP=$(CCSLIB)/minabi/libp
DONE=
PROF=
NONPROF=
PIC=
ABI=
DEFLIST=
OWN=bin
GRP=bin
SGSBASE=../../cmd/sgs
ISANSI=TRUE
RTLD_DIR=../rtld

all:	all_objects
	if u3b15 || u3b5 || u3b2 ;  \
	then $(MAKE) -e -f libc.mk DEFLIST=-DM32 ISANSI=$$ISANSI \
				archive_lib shared_lib abi_lib MACHINE=m32 ; \
	elif i386 ;  \
	then $(MAKE) -e -f libc.mk DEFLIST=-Di386 ISANSI=$$ISANSI \
				archive_lib shared_lib abi_lib MACHINE=i386 ; fi

archive:	archive_objects
	if u3b15 || u3b5 || u3b2 ;  \
	then $(MAKE) -e -f libc.mk DEFLIST=-DM32 ISANSI=$$ISANSI \
				archive_lib MACHINE=m32 ; \
	elif i386 ;  \
	then $(MAKE) -e -f libc.mk DEFLIST=-Di386 ISANSI=$$ISANSI \
				archive_lib MACHINE=i386 ; fi

shared:		shared_objects
	if u3b15 || u3b5 || u3b2 ;  \
	then $(MAKE) -e -f libc.mk DEFLIST=-DM32 ISANSI=$$ISANSI \
				shared_lib MACHINE=m32 ; \
	elif i386 ;  \
	then $(MAKE) -e -f libc.mk DEFLIST=-Di386 ISANSI=$$ISANSI \
				shared_lib MACHINE=i386 ; fi

abi:	abi_objects
	if u3b15 || u3b5 || u3b2 ;  \
	then $(MAKE) -e -f libc.mk DEFLIST=-DM32 ISANSI=$$ISANSI \
				abi_lib MACHINE=m32 ; \
	elif i386 ;  \
	then $(MAKE) -e -f libc.mk DEFLIST=-Di386 ISANSI=$$ISANSI \
				abi_lib MACHINE=i386 ; fi

all_objects:	rtld
	- $(CC) -E tstansi.c; if [ $$? = 0 ] ; \
	then ISANSI="TRUE"; \
	else ISANSI="FALSE"; PIC="@#"; ABI="@#"; \
	fi ; \
	if u3b15 || u3b5 || u3b2 ;  \
	then $(MAKE) -e -f libc.mk DEFLIST=-DM32 ISANSI=$$ISANSI \
				specific MACHINE=m32 ; \
	elif i386 ;  \
	then $(MAKE) -e -f libc.mk DEFLIST=-Di386 ISANSI=$$ISANSI \
				specific MACHINE=i386 ; fi

shared_objects:	rtld
	- $(CC) -E tstansi.c; if [ $$? = 0 ] ; \
	then ISANSI="TRUE"; ABI="@#"; \
	else ISANSI="FALSE"; PIC="@#"; ABI="@#"; \
	fi ; \
	if u3b15 || u3b5 || u3b2 ;  \
	then $(MAKE) -e -f libc.mk DEFLIST=-DM32 ISANSI=$$ISANSI PIC=$$PIC \
				ABI=$$ABI specific MACHINE=m32 ; \
	elif i386 ;  \
	then $(MAKE) -e -f libc.mk DEFLIST=-Di386 ISANSI=$$ISANSI PIC=$$PIC \
				ABI=$$ABI specific MACHINE=i386 ; fi

abi_objects:	rtld
	- $(CC) -E tstansi.c; if [ $$? = 0 ] ; \
	then ISANSI="TRUE"; PIC="@#"; \
	else ISANSI="FALSE"; PIC="@#"; ABI="@#"; \
	fi ; \
	if u3b15 || u3b5 || u3b2 ;  \
	then $(MAKE) -e -f libc.mk DEFLIST=-DM32 ISANSI=$$ISANSI PIC=$$PIC \
				ABI=$$ABI specific MACHINE=m32 ; \
	elif i386 ;  \
	then $(MAKE) -e -f libc.mk DEFLIST=-Di386 ISANSI=$$ISANSI PIC=$$PIC \
				ABI=$$ABI specific MACHINE=i386 ; fi

archive_objects:
	- $(CC) -E tstansi.c; if [ $$? = 0 ] ; \
	then ISANSI="TRUE"; PIC="@#"; ABI="@#"; \
	else ISANSI="FALSE"; PIC="@#"; ABI="@#"; \
	fi ; \
	if u3b15 || u3b5 || u3b2 ;  \
	then $(MAKE) -e -f libc.mk DEFLIST=-DM32 ISANSI=$$ISANSI PIC=$$PIC \
				ABI=$$ABI specific MACHINE=m32 ; \
	elif i386 ;  \
	then $(MAKE) -e -f libc.mk DEFLIST=-Di386 ISANSI=$$ISANSI PIC=$$PIC \
				ABI=$$ABI specific MACHINE=i386 ; fi

specific:
	#
	# compile portable library modules
	cd port; $(MAKE) -f makefile -e CC=$(CC)
	#
	# compile machine-dependent library modules
	cd $(MACHINE); $(MAKE) -f makefile -e CC=$(CC) ISANSI=$(ISANSI)

rtld:
	# make the rtld objects
	cd $(RTLD_DIR); $(MAKE) -f rtld.mk $(ENVPARMS)

archive_lib:
	#
	# place portable modules in "object" directory, then overlay
	# 	the machine-dependent modules.
	-rm -rf object
	mkdir object
	find port $(MACHINE) -name '*.o' -print | \
	xargs sh -sc 'cp "$$@" object'
	$(PROF)find port $(MACHINE) -name '*.p' -print | \
	xargs sh -sc 'cp "$$@" object'
	#
	# delete temporary libraries
	-rm -f lib.libc
	$(PROF)-rm -f libp.libc
	#
	# set aside run-time modules, which don't go in library archive!
	cd object; for i in *crt?.o values-Xt.o values-Xa.o values-Xc.o; do mv $$i ..; done
	#
	# build archive out of the remaining modules.
	cd object; $(MAKE) -e -f ../$(MACHINE)/makefile archive \
		AR=$(AR)  LORDER=$(LORDER) PROF=$(PROF) MAC=$(MAC) \
		ISANSI=$(ISANSI)
	-rm -rf object
	#
	$(DONE)

shared_lib:
	#
	# place portable modules in "object" directory, then overlay
	# 	the machine-dependent modules.
	-rm -rf object
	mkdir object
	cp port/*/[_a-m]*.o object
	cp port/*/[n-z]*.o object
	cp port/*/[_a-m]*.P object
	cp port/*/[n-z]*.P object
	cp $(MACHINE)/*/[_a-m]*.o object
	cp $(MACHINE)/*/[n-z]*.o object
	cp $(MACHINE)/*/[_a-m]*.P object
	cp $(MACHINE)/*/[n-z]*.P object
	cp $(RTLD_DIR)/$(MACHINE)/*.o object
	#
	# delete temporary libraries
	-rm -f libc.so
	#
	# set aside run-time modules, which don't go in library archive!
	cd object; for i in *crt?.o values-Xa.o values-Xc.o; do mv $$i ..; done; \
	cp values-Xt.o ..
	#
	# build archive out of the remaining modules.
	cd object; $(MAKE) -e -f ../$(MACHINE)/makefile shared \
		AR=$(AR)  LORDER=$(LORDER) PROF=$(PROF) MAC=$(MAC) \
		ISANSI=$(ISANSI)
	-rm -rf object
	#
	$(DONE)

abi_lib:
	#
	# place portable modules in "object" directory, then overlay
	# 	the machine-dependent modules.
	-rm -rf object
	mkdir object
	cp port/*/[_a-m]*.o object
	cp port/*/[n-z]*.o object
	cp port/*/[_a-m]*.A object
	cp port/*/[n-z]*.A object
	cp $(MACHINE)/*/[_a-m]*.o object
	cp $(MACHINE)/*/[n-z]*.o object
	cp $(MACHINE)/*/[_a-m]*.A object
	cp $(MACHINE)/*/[n-z]*.A object
	cp $(RTLD_DIR)/$(MACHINE)/*.o object
	#
	# delete temporary libraries
	-rm -f libabi.so
	#
	# set aside run-time modules, which don't go in library archive!
	cd object; for i in *crt?.o values-Xt.o values-Xa.o values-Xc.o; do mv $$i ..; done
	#
	# build archive out of the remaining modules.
	cd object; $(MAKE) -e -f ../$(MACHINE)/makefile abi_lib \
		AR=$(AR)  LORDER=$(LORDER) PROF=$(PROF) MAC=$(MAC) \
		ISANSI=$(ISANSI)
	-rm -rf object
	#
	$(DONE)

move:	move_archive
	#
	# move the shared and abi libraries into the correct directories
	sh $(SGSBASE)/sgs.install 644 $(OWN) $(GRP) $(CCSLIB)/lib$(VARIANT)c.so libc.so ; \
	rm -f libc.so
	sh $(SGSBASE)/sgs.install 755 $(OWN) $(GRP) $(USRLIB)/lib$(VARIANT)c.so.1 libc.so.1 ; \
	rm -f libc.so.1
	rm -f $(LIBP)/lib$(VARIANT)c.so
	ln $(CCSLIB)/lib$(VARIANT)c.so $(LIBP)/lib$(VARIANT)c.so

	if [ ! -d $(ABILIB) ]; then \
	mkdir $(ABILIB); \
	fi
	sh $(SGSBASE)/sgs.install 644 $(OWN) $(GRP) $(ABILIB)/lib$(VARIANT)c.so libabi.so ; \
	rm -f libabi.so
	sh $(SGSBASE)/sgs.install 755 $(OWN) $(GRP) $(USRLIB)/ld.so.1 ld.so.1 ; \
	rm -f ld.so.1
	if [ ! -d $(ABILIBP) ]; then \
	mkdir $(ABILIBP); \
	fi
	rm -f $(ABILIBP)/lib$(VARIANT)c.so
	ln $(ABILIB)/lib$(VARIANT)c.so $(ABILIBP)/lib$(VARIANT)c.so

move_archive:
	#
	# move the library or libraries into the correct directory
	for i in *crt?.o values-Xt.o values-Xa.o values-Xc.o;  \
	do sh $(SGSBASE)/sgs.install 644 $(OWN) $(GRP) $(CCSLIB)/$(SGS)$$i $$i; \
	rm -f $$i ; done
	sh $(SGSBASE)/sgs.install 644 $(OWN) $(GRP) $(CCSLIB)/lib$(VARIANT)c.a lib.libc ; \
	rm -f lib.libc
	$(PROF) if [ ! -d $(LIBP) ]; then \
	$(PROF) mkdir $(LIBP); \
	$(PROF) fi
	$(PROF)sh $(SGSBASE)/sgs.install 644 $(OWN) $(GRP) $(LIBP)/lib$(VARIANT)c.a libp.libc ; \
	rm -f libp.libc

install:	all move

install_archive:	archive move_archive

clean:
	#
	# remove intermediate files except object modules and temp library
	-rm -rf obj*
	cd port ;  $(MAKE) clean
	if u3b15 || u3b5 || u3b2 ;   then cd m32 ;   $(MAKE) clean ; fi
	if i386 ;  then cd i386 ;  $(MAKE) clean ; fi

clobber:
	#
	# remove intermediate files
	-rm -rf *.o lib*.libc obj*
	-rm -rf *.o libc.so libabi.so libc.so.1 ld.so.1
	cd port ;  $(MAKE) clobber
	if u3b15 || u3b5 || u3b2 ;   then cd m32 ;   $(MAKE) clobber ; fi
	if i386 ;  then cd i386 ;  $(MAKE) clobber ; fi
